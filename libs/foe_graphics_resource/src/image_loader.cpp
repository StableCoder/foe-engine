// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/image_loader.hpp>

#include <FreeImage.h>
#include <foe/ecs/id_to_string.hpp>
#include <foe/graphics/resource/image_create_info.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/vk/format.hpp>
#include <foe/graphics/vk/image.hpp>
#include <foe/graphics/vk/session.hpp>

#include "log.hpp"
#include "result.h"
#include "vk_result.h"

foeResultSet foeImageLoader::initialize(
    foeResourcePool resourcePool,
    std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn) {
    if (resourcePool == FOE_NULL_HANDLE || !externalFileSearchFn)
        return to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_IMAGE_LOADER_INITIALIZATION_FAILED);

    mResourcePool = resourcePool;
    mExternalFileSearchFn = externalFileSearchFn;

    return to_foeResult(FOE_GRAPHICS_RESOURCE_SUCCESS);
}

void foeImageLoader::deinitialize() {
    mExternalFileSearchFn = {};
    mResourcePool = FOE_NULL_HANDLE;
}

bool foeImageLoader::initialized() const noexcept { return !!mExternalFileSearchFn; }

foeResultSet foeImageLoader::initializeGraphics(foeGfxSession gfxSession) {
    if (!initialized()) {
        return to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_IMAGE_LOADER_NOT_INITIALIZED);
    }

    mGfxSession = gfxSession;

    foeResultSet result = foeGfxCreateUploadContext(gfxSession, &mGfxUploadContext);
    if (result.value != FOE_SUCCESS) {
        deinitializeGraphics();
    }

    return result;
}

void foeImageLoader::deinitializeGraphics() {
    // Unload all resources this loader loaded
    bool upcomingWork;
    do {
        upcomingWork = foeResourcePoolUnloadType(mResourcePool,
                                                 FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE) > 0;

        gfxMaintenance();

        mLoadSync.lock();
        upcomingWork |= !mLoadRequests.empty();
        mLoadSync.unlock();

        mUnloadSync.lock();
        upcomingWork |= !mUnloadRequests.empty();
        mUnloadSync.unlock();

        mDestroySync.lock();
        for (auto const &it : mDataDestroyLists) {
            upcomingWork |= !it.empty();
        }
        mDestroySync.unlock();
    } while (upcomingWork);

    // Internal
    if (mGfxUploadContext != FOE_NULL_HANDLE)
        foeGfxDestroyUploadContext(mGfxUploadContext);
    mGfxUploadContext = FOE_NULL_HANDLE;

    // External
    mGfxSession = FOE_NULL_HANDLE;
}

bool foeImageLoader::initializedGraphics() const noexcept { return mGfxSession != FOE_NULL_HANDLE; }

void foeImageLoader::gfxMaintenance() {
    // Previous Data Destruction
    mDestroySync.lock();
    ++mDataDestroyIndex;
    if (mDataDestroyIndex >= mDataDestroyLists.size()) {
        mDataDestroyIndex = 0;
    }
    auto toDestroy = std::move(mDataDestroyLists[mDataDestroyIndex]);
    mDestroySync.unlock();

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
    mUnloadSync.lock();
    auto toUnload = std::move(mUnloadRequests);
    mUnloadSync.unlock();

    for (auto &it : toUnload) {
        // Unload the resource, adding it'd data for later destruction
        unloadResource(this, it.resource, it.iteration, it.pUnloadCallFn, true);
        foeResourceDecrementRefCount(it.resource);
    }

    // Process Loads
    mLoadSync.lock();
    auto toLoad = std::move(mLoadRequests);
    mLoadSync.unlock();

    std::vector<LoadData> stillLoading;
    stillLoading.reserve(toLoad.size() / 2);

    for (auto &it : toLoad) {
        // Check to see if this one has completed uploading
        auto requestStatus = foeGfxGetUploadRequestStatus(it.uploadRequest);

        if (requestStatus == FOE_GFX_UPLOAD_REQUEST_STATUS_COMPLETE) {
            // It completed the upload, move the data

            if (it.resource != FOE_NULL_HANDLE) {
                // If we have an image to update, do so
                auto moveFn = [](void *pSrc, void *pDst) {
                    auto *pSrcData = (foeImage *)pSrc;
                    new (pDst) foeImage(std::move(*pSrcData));
                };

                it.pPostLoadFn(it.resource, {}, &it.data, moveFn, it.createInfo, this,
                               foeImageLoader::unloadResource);
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
            it.pPostLoadFn(it.resource,
                           to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_IMAGE_UPLOAD_FAILURE), nullptr,
                           nullptr, nullptr, nullptr, nullptr);
        } else {
            stillLoading.emplace_back(std::move(it));
        }
    }

    if (!stillLoading.empty()) {
        mLoadSync.lock();

        mLoadRequests.reserve(mLoadRequests.size() + stillLoading.size());

        for (auto &it : stillLoading) {
            mLoadRequests.emplace_back(std::move(it));
        }

        mLoadSync.unlock();
    }
}

bool foeImageLoader::canProcessCreateInfo(foeResourceCreateInfo createInfo) {
    return foeResourceCreateInfoGetType(createInfo) ==
           FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
}

void foeImageLoader::load(void *pLoader,
                          foeResource resource,
                          foeResourceCreateInfo createInfo,
                          PFN_foeResourcePostLoad *pPostLoadFn) {
    reinterpret_cast<foeImageLoader *>(pLoader)->load(resource, createInfo, pPostLoadFn);
}

void foeImageLoader::load(foeResource resource,
                          foeResourceCreateInfo createInfo,
                          PFN_foeResourcePostLoad *pPostLoadFn) {
    if (!canProcessCreateInfo(createInfo)) {
        pPostLoadFn(resource, to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_INCOMPATIBLE_CREATE_INFO),
                    nullptr, nullptr, nullptr, nullptr, nullptr);
        return;
    }
    auto const *pImageCI = (foeImageCreateInfo const *)foeResourceCreateInfoGetData(createInfo);

    foeResultSet result = to_foeResult(FOE_GRAPHICS_RESOURCE_SUCCESS);
    VkResult vkRes{VK_SUCCESS};
    foeGfxUploadRequest gfxUploadRequest{FOE_NULL_HANDLE};
    foeGfxUploadBuffer gfxUploadBuffer{FOE_NULL_HANDLE};
    foeImage imgData{};

    { // Import the data
      // Find the file path first
        std::filesystem::path filePath = mExternalFileSearchFn(pImageCI->pFile);
        // Determine the image format
        FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFileType(filePath.string().c_str());
        if (imageFormat == FIF_UNKNOWN) {
            FreeImage_GetFIFFromFilename(filePath.string().c_str());
        }
        if (imageFormat == FIF_UNKNOWN) {
            FOE_LOG(foeGraphicsResource, Error, "Could not determine image format for: {}",
                    filePath.string())
            result = to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_EXTERNAL_IMAGE_FORMAT_UNKNOWN);
            goto LOADING_FAILED;
        }

        // Load the image into memory
        auto *bitmap = FreeImage_Load(imageFormat, filePath.string().c_str(), 0);
        if (bitmap == nullptr) {
            FOE_LOG(foeGraphicsResource, Error, "Failed to load image: {}", filePath.string())
            result = to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_EXTERNAL_IMAGE_LOAD_FAILURE);
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
            result = foeGfxCreateUploadBuffer(mGfxUploadContext, totalDataSize, &gfxUploadBuffer);
            if (result.value != FOE_SUCCESS)
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

                result = foeGfxMapUploadBuffer(mGfxUploadContext, gfxUploadBuffer,
                                               reinterpret_cast<void **>(&pData));
                if (result.value != FOE_SUCCESS)
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

                result = recordImageUploadCommands(
                    mGfxUploadContext, &subresourceRange, static_cast<uint32_t>(copyRegions.size()),
                    copyRegions.data(), gfxUploadBuffer, imgData.image, VK_ACCESS_SHADER_READ_BIT,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &gfxUploadRequest);
                if (result.value != FOE_SUCCESS) {
                    goto LOADING_FAILED;
                }

                result = foeSubmitUploadDataCommands(mGfxUploadContext, gfxUploadRequest);
                if (result.value != FOE_SUCCESS) {
                    goto LOADING_FAILED;
                }
            }
        }
    }

LOADING_FAILED:
    if (result.value == FOE_SUCCESS && vkRes != VK_SUCCESS) {
        result = vk_to_foeResult(vkRes);
    }

    if (result.value != FOE_SUCCESS) {
        // Failed at some point, clear all relevant data
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeGraphicsResource, Error, "Failed to load foeImage {} with error {}",
                foeIdToString(foeResourceGetID(resource)), buffer)

        // Run the post-load function with the error
        pPostLoadFn(resource, result, nullptr, nullptr, nullptr, nullptr, nullptr);

        if (gfxUploadRequest != FOE_NULL_HANDLE) {
            // A partial upload success, leave pimage an nullptr, so the upload completes then the
            // data is safely destroyed
            mLoadSync.lock();
            mLoadRequests.emplace_back(LoadData{
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
        mLoadRequests.emplace_back(LoadData{
            .resource = resource,
            .createInfo = createInfo,
            .pPostLoadFn = pPostLoadFn,
            .data = std::move(imgData),
            .uploadRequest = gfxUploadRequest,
            .uploadBuffer = gfxUploadBuffer,
        });
        mLoadSync.unlock();
    }
}

void foeImageLoader::unloadResource(void *pContext,
                                    foeResource resource,
                                    uint32_t resourceIteration,
                                    PFN_foeResourceUnloadCall *pUnloadCallFn,
                                    bool immediateUnload) {
    auto *pLoader = reinterpret_cast<foeImageLoader *>(pContext);

    if (immediateUnload) {
        auto moveFn = [](void *pSrc, void *pDst) {
            auto *pSrcData = (foeImage *)pSrc;
            auto *pDstData = (foeImage *)pDst;

            *pDstData = std::move(*pSrcData);
            pSrcData->~foeImage();
        };

        foeImage data{};

        if (pUnloadCallFn(resource, resourceIteration, &data, moveFn)) {
            pLoader->mDestroySync.lock();
            pLoader->mDataDestroyLists[pLoader->mDataDestroyIndex].emplace_back(std::move(data));
            pLoader->mDestroySync.unlock();
        }

    } else {
        pLoader->mUnloadSync.lock();

        pLoader->mUnloadRequests.emplace_back(UnloadData{
            .resource = resource,
            .iteration = resourceIteration,
            .pUnloadCallFn = pUnloadCallFn,
        });

        pLoader->mUnloadSync.unlock();
    }
}
