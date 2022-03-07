/*
    Copyright (C) 2021-2022 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <foe/graphics/resource/image_loader.hpp>

#include <FreeImage.h>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/vk/format.hpp>
#include <foe/graphics/vk/image.hpp>
#include <foe/graphics/vk/session.hpp>
#include <vk_error_code.hpp>

#include "error_code.hpp"
#include "log.hpp"

std::error_code foeImageLoader::initialize(
    std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn) {
    if (!externalFileSearchFn)
        return FOE_GRAPHICS_RESOURCE_ERROR_IMAGE_LOADER_INITIALIZATION_FAILED;

    mExternalFileSearchFn = externalFileSearchFn;

    return FOE_GRAPHICS_RESOURCE_SUCCESS;
}

void foeImageLoader::deinitialize() { mExternalFileSearchFn = {}; }

bool foeImageLoader::initialized() const noexcept { return !!mExternalFileSearchFn; }

auto foeImageLoader::initializeGraphics(foeGfxSession gfxSession) -> std::error_code {
    if (!initialized()) {
        return FOE_GRAPHICS_RESOURCE_ERROR_IMAGE_LOADER_NOT_INITIALIZED;
    }

    mGfxSession = gfxSession;

    std::error_code errC = foeGfxCreateUploadContext(gfxSession, &mGfxUploadContext);
    if (errC) {
        deinitializeGraphics();
    }

    return errC;
}

void foeImageLoader::deinitializeGraphics() {
    if (mGfxUploadContext != FOE_NULL_HANDLE)
        foeGfxDestroyUploadContext(mGfxUploadContext);
    mGfxUploadContext = FOE_NULL_HANDLE;

    mGfxSession = FOE_NULL_HANDLE;
}

bool foeImageLoader::initializedGraphics() const noexcept { return mGfxSession != FOE_NULL_HANDLE; }

void foeImageLoader::gfxMaintenance() {
    // Previous Data Destruction
    ++mDataDestroyIndex;
    if (mDataDestroyIndex >= mDataDestroyLists.size()) {
        mDataDestroyIndex = 0;
    }

    auto toDestroy = std::move(mDataDestroyLists[mDataDestroyIndex]);
    for (auto &it : toDestroy) {
        VkDevice device = foeGfxVkGetDevice(mGfxSession);
        VmaAllocator allocator = foeGfxVkGetAllocator(mGfxSession);

        if (it.sampler != VK_NULL_HANDLE)
            vkDestroySampler(device, it.sampler, nullptr);

        if (it.view != VK_NULL_HANDLE)
            vkDestroyImageView(device, it.view, nullptr);

        if (it.image != VK_NULL_HANDLE)
            vmaDestroyImage(allocator, it.image, it.alloc);
    }

    // Process Unloads
    mUnloadRequestsSync.lock();
    auto toUnload = std::move(mUnloadRequests);
    mUnloadRequestsSync.unlock();

    for (auto &it : toUnload) {
        // Unload the resource, adding it'd data for later destruction
        unloadResource(this, it.pImage, it.iteration, true);
    }

    // Process Loads
    mLoadSync.lock();
    auto toLoad = std::move(mToLoad);
    mLoadSync.unlock();

    std::vector<LoadData> stillLoading;
    stillLoading.reserve(toLoad.size() / 2);

    for (auto &it : toLoad) {
        // Check to see if this one has completed uploading
        auto requestStatus = foeGfxGetUploadRequestStatus(it.uploadRequest);

        if (requestStatus == FOE_GFX_UPLOAD_REQUEST_STATUS_COMPLETE) {
            // It completed the upload, move the data

            if (it.pImage) {
                // If we have an image to update, do so
                it.pImage->modifySync.lock();

                if (it.pImage->data.pUnloadFn != nullptr) {
                    it.pImage->data.pUnloadFn(it.pImage->data.pUnloadContext, it.pImage,
                                              it.pImage->iteration, true);
                }

                ++it.pImage->iteration;
                it.pImage->data = std::move(it.data);
                it.pPostLoadFn(it.pImage, {});

                it.pImage->modifySync.unlock();
            } else {
                // Destroy the data immediately
                VkDevice device = foeGfxVkGetDevice(mGfxSession);
                VmaAllocator allocator = foeGfxVkGetAllocator(mGfxSession);

                if (it.data.sampler != VK_NULL_HANDLE)
                    vkDestroySampler(device, it.data.sampler, nullptr);
                if (it.data.view != VK_NULL_HANDLE)
                    vkDestroyImageView(device, it.data.view, nullptr);
                if (it.data.image != VK_NULL_HANDLE)
                    vmaDestroyImage(allocator, it.data.image, it.data.alloc);
            }

            if (it.uploadBuffer != FOE_NULL_HANDLE)
                foeGfxDestroyUploadBuffer(mGfxUploadContext, it.uploadBuffer);
            foeGfxDestroyUploadRequest(mGfxUploadContext, it.uploadRequest);
        } else if (requestStatus != FOE_GFX_UPLOAD_REQUEST_STATUS_INCOMPLETE) {
            // There's an error, this is lost.
            if (it.pImage != nullptr) {
                it.pImage->state = foeResourceState::Failed;
            }
        } else {
            stillLoading.emplace_back(std::move(it));
        }
    }

    if (!stillLoading.empty()) {
        mLoadSync.lock();

        mToLoad.reserve(mToLoad.size() + stillLoading.size());

        for (auto &it : stillLoading) {
            mToLoad.emplace_back(std::move(it));
        }

        mLoadSync.unlock();
    }
}

bool foeImageLoader::canProcessCreateInfo(foeResourceCreateInfoBase *pCreateInfo) {
    return dynamic_cast<foeImageCreateInfo *>(pCreateInfo) != nullptr;
}

void foeImageLoader::load(void *pLoader,
                          void *pResource,
                          std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
                          void (*pPostLoadFn)(void *, std::error_code)) {
    reinterpret_cast<foeImageLoader *>(pLoader)->load(pResource, pCreateInfo, pPostLoadFn);
}

void foeImageLoader::load(void *pResource,
                          std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
                          void (*pPostLoadFn)(void *, std::error_code)) {
    auto *pImage = reinterpret_cast<foeImage *>(pResource);
    auto *pImageCI = dynamic_cast<foeImageCreateInfo *>(pCreateInfo.get());

    if (pImageCI == nullptr) {
        pPostLoadFn(pResource, FOE_GRAPHICS_RESOURCE_ERROR_INCOMPATIBLE_CREATE_INFO);
        return;
    }

    std::error_code errC;
    VkResult vkRes{VK_SUCCESS};
    foeGfxUploadRequest gfxUploadRequest{FOE_NULL_HANDLE};
    foeGfxUploadBuffer gfxUploadBuffer{FOE_NULL_HANDLE};
    foeImage::Data imgData{
        .pUnloadContext = this,
        .pUnloadFn = unloadResource,
        .pCreateInfo = pCreateInfo,
    };

    { // Import the data
      // Find the file path first
        std::filesystem::path filePath = mExternalFileSearchFn(pImageCI->fileName);
        // Determine the image format
        FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFileType(filePath.string().c_str());
        if (imageFormat == FIF_UNKNOWN) {
            FreeImage_GetFIFFromFilename(filePath.string().c_str());
        }
        if (imageFormat == FIF_UNKNOWN) {
            FOE_LOG(foeGraphicsResource, Error, "Could not determine image format for: {}",
                    filePath.string())
            errC = FOE_GRAPHICS_RESOURCE_ERROR_EXTERNAL_IMAGE_FORMAT_UNKNOWN;
            goto LOADING_FAILED;
        }

        // Load the image into memory
        auto *bitmap = FreeImage_Load(imageFormat, filePath.string().c_str(), 0);
        if (bitmap == nullptr) {
            FOE_LOG(foeGraphicsResource, Error, "Failed to load image: {}", filePath.string())
            errC = FOE_GRAPHICS_RESOURCE_ERROR_EXTERNAL_IMAGE_LOAD_FAILURE;
            goto LOADING_FAILED;
        }

        { // Convert to 32 bit RGBA
            auto *newBitmap = FreeImage_ConvertTo32Bits(bitmap);
            // Unload original
            FreeImage_Unload(bitmap);
            bitmap = newBitmap;
        }

        VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
        VkExtent3D extent{
            .width = FreeImage_GetWidth(bitmap),
            .height = FreeImage_GetHeight(bitmap),
            .depth = 1,
        };
        auto mipLevels = maxMipmapCount(extent);
        auto bpp = foeGfxVkBytesPerPixel(format, VK_IMAGE_ASPECT_COLOR_BIT);
        size_t totalDataSize = pixelCount(extent, mipLevels) * bpp;

        std::unique_ptr<uint8_t[]> pelData(new uint8_t[totalDataSize]);
        std::vector<uint8_t *> mipmapOffsetPtrs;

        auto *pPelData = pelData.get();
        for (uint32_t m = 0; m < mipLevels; ++m) {
            auto mipExtent = mipmapExtent(extent, m);
            auto mipDataSize = bpp * mipExtent.width * mipExtent.height * mipExtent.depth;

            mipmapOffsetPtrs.emplace_back(pPelData);

            auto *mipBitmap = FreeImage_Rescale(bitmap, mipExtent.width, mipExtent.height);
            auto *mipData = FreeImage_GetBits(mipBitmap);

            std::memcpy(pPelData, mipData, mipDataSize);
            pPelData += mipDataSize;

            FreeImage_Unload(mipBitmap);
        }

        FreeImage_Unload(bitmap);

        // Create the resources
        { // Staging Buffer
            errC = foeGfxCreateUploadBuffer(mGfxUploadContext, totalDataSize, &gfxUploadBuffer);
            if (errC)
                goto LOADING_FAILED;
        }

        { // Image
            VkImageCreateInfo imageCI{
                .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .imageType = VK_IMAGE_TYPE_2D,
                .format = format,
                .extent = extent,
                .mipLevels = mipLevels,
                .arrayLayers = 1U,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .tiling = VK_IMAGE_TILING_OPTIMAL,
                .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            };

            VmaAllocationCreateInfo allocCI{
                .usage = VMA_MEMORY_USAGE_GPU_ONLY,
            };

            vkRes = vmaCreateImage(foeGfxVkGetAllocator(mGfxSession), &imageCI, &allocCI,
                                   &imgData.image, &imgData.alloc, nullptr);
            if (vkRes != VK_SUCCESS) {
                goto LOADING_FAILED;
            }
        }

        { // Image View
            VkImageViewCreateInfo viewCI{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = imgData.image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = format,
                .components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
                               VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A},
                .subresourceRange =
                    VkImageSubresourceRange{
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = 0,
                        .levelCount = mipLevels,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                    },
            };

            vkRes =
                vkCreateImageView(foeGfxVkGetDevice(mGfxSession), &viewCI, nullptr, &imgData.view);
            if (vkRes != VK_SUCCESS) {
                goto LOADING_FAILED;
            }
        }

        { // Sampler
            VkSamplerCreateInfo samplerCI{
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = VK_FILTER_NEAREST,
                .minFilter = VK_FILTER_NEAREST,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .mipLodBias = 0.f,
                .anisotropyEnable = VK_FALSE,
                .minLod = 0.f,
                .maxLod = static_cast<float>(mipLevels - 1),
            };

            vkRes = vkCreateSampler(foeGfxVkGetDevice(mGfxSession), &samplerCI, nullptr,
                                    &imgData.sampler);
            if (vkRes != VK_SUCCESS) {
                goto LOADING_FAILED;
            }
        }

        // Start the upload process
        {
            std::vector<VkBufferImageCopy> copyRegions;
            { // Map data in
                uint8_t *pData;
                VkDeviceSize offset = 0;
                copyRegions.resize(mipLevels);

                errC = foeGfxMapUploadBuffer(mGfxUploadContext, gfxUploadBuffer,
                                             reinterpret_cast<void **>(&pData));
                if (errC)
                    goto LOADING_FAILED;

                for (uint32_t i = 0; i < mipLevels; ++i) {
                    auto mipExtent = mipmapExtent(extent, i);

                    copyRegions[i] = VkBufferImageCopy{
                        .bufferOffset = offset,
                        .imageSubresource =
                            {
                                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                .mipLevel = i,
                                .baseArrayLayer = 0,
                                .layerCount = 1,
                            },
                        .imageExtent = mipExtent,
                    };

                    std::memcpy(pData + offset, mipmapOffsetPtrs[i],
                                bpp * mipExtent.width * mipExtent.height * mipExtent.depth);

                    offset += mipExtent.width * mipExtent.height * mipExtent.depth *
                              foeGfxVkBytesPerPixel(format, VK_IMAGE_ASPECT_COLOR_BIT);
                }

                foeGfxUnmapUploadBuffer(mGfxUploadContext, gfxUploadBuffer);
            }

            { // Record and submit upload commands
                VkImageSubresourceRange subresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = mipLevels,
                    .layerCount = 1,
                };

                vkRes = recordImageUploadCommands(
                    mGfxUploadContext, &subresourceRange, static_cast<uint32_t>(copyRegions.size()),
                    copyRegions.data(), gfxUploadBuffer, imgData.image, VK_ACCESS_SHADER_READ_BIT,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &gfxUploadRequest);
                if (vkRes != VK_SUCCESS) {
                    goto LOADING_FAILED;
                }

                errC = foeSubmitUploadDataCommands(mGfxUploadContext, gfxUploadRequest);
                if (errC) {
                    vkRes = static_cast<VkResult>(errC.value());
                    goto LOADING_FAILED;
                }
            }
        }
    }

LOADING_FAILED:
    if (!errC && vkRes != VK_SUCCESS) {
        errC = vkRes;
    }
    if (errC) {
        // Failed at some point, clear all relevant data
        FOE_LOG(foeGraphicsResource, Error, "Failed to load foeImage {} with error {}:{}",
                foeIdToString(pImage->getID()), errC.value(), errC.message())

        // Run the post-load function with the error
        pPostLoadFn(pImage, errC);

        if (gfxUploadRequest != FOE_NULL_HANDLE) {
            // A partial upload success, leave pimage an nullptr, so the upload completes then the
            // data is safely destroyed
            mLoadSync.lock();
            mToLoad.emplace_back(LoadData{
                .pImage = nullptr,
                .pPostLoadFn = nullptr,
                .data = std::move(imgData),
                .uploadRequest = gfxUploadRequest,
                .uploadBuffer = gfxUploadBuffer,
            });
            mLoadSync.unlock();
        } else {
            // The upload never even began, destroy the data now
            VkDevice device = foeGfxVkGetDevice(mGfxSession);
            VmaAllocator allocator = foeGfxVkGetAllocator(mGfxSession);

            if (imgData.sampler != VK_NULL_HANDLE)
                vkDestroySampler(device, imgData.sampler, nullptr);
            if (imgData.view != VK_NULL_HANDLE)
                vkDestroyImageView(device, imgData.view, nullptr);
            if (imgData.image != VK_NULL_HANDLE)
                vmaDestroyImage(allocator, imgData.image, imgData.alloc);
        }
    } else {
        // Successfully processed and is being uploaded now
        mLoadSync.lock();
        mToLoad.emplace_back(LoadData{
            .pImage = pImage,
            .pPostLoadFn = pPostLoadFn,
            .data = std::move(imgData),
            .uploadRequest = gfxUploadRequest,
            .uploadBuffer = gfxUploadBuffer,
        });
        mLoadSync.unlock();
    }
}

void foeImageLoader::unloadResource(void *pContext,
                                    void *pResource,
                                    uint32_t resourceIteration,
                                    bool immediateUnload) {
    auto *pLoader = reinterpret_cast<foeImageLoader *>(pContext);
    auto *pImage = reinterpret_cast<foeImage *>(pResource);

    if (immediateUnload) {
        pImage->modifySync.lock();

        if (pImage->iteration == resourceIteration) {
            pLoader->mDataDestroyLists[pLoader->mDataDestroyIndex].emplace_back(
                std::move(pImage->data));
            pImage->data = {};
            pImage->state = foeResourceState::Unloaded;
            ++pImage->iteration;
        }

        pImage->modifySync.unlock();
    } else {
        pLoader->mUnloadRequestsSync.lock();

        pLoader->mUnloadRequests.emplace_back(UnloadData{
            .pImage = pImage,
            .iteration = resourceIteration,
        });

        pLoader->mUnloadRequestsSync.unlock();
    }
}
