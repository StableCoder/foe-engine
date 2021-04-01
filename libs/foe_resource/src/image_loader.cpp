/*
    Copyright (C) 2021 George Cave.

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

#include <foe/resource/image_loader.hpp>

#include <foe/graphics/vk/session.hpp>

#include "error_code.hpp"
#include "log.hpp"

foeImageLoader::~foeImageLoader() {
    if (mActiveJobs > 0) {
        FOE_LOG(foeResource, Fatal, "foeImageLoader being destructed with {} active jobs!",
                mActiveJobs.load());
    }
    if (mActiveUploads > 0) {
        FOE_LOG(foeResource, Fatal,
                "foeImageLoader being destructed with {} active uploads in progress!",
                mActiveUploads.load());
    }
}

std::error_code foeImageLoader::initialize(
    foeGfxSession session,
    std::function<bool(foeResourceID, foeImageCreateInfo &)> importFunction,
    std::function<void(std::function<void()>)> asynchronousJobs) {
    if (initialized()) {
        return FOE_RESOURCE_ERROR_ALREADY_INITIALIZED;
    }

    std::error_code errC{FOE_RESOURCE_SUCCESS};

    mGfxSession = session;
    mImportFunction = importFunction;
    mAsyncJobs = asynchronousJobs;

    errC = foeGfxCreateUploadContext(session, &mGfxUploadContext);
    if (errC)
        goto INITIALIZATION_FAILED;

INITIALIZATION_FAILED:
    if (errC) {
        deinitialize();
    }

    return errC;
}

void foeImageLoader::deinitialize() {
    if (mActiveJobs > 0) {
        FOE_LOG(foeResource, Fatal, "foeImageLoader being deinitialized with {} active jobs!",
                mActiveJobs.load());
    }
    if (mActiveUploads > 0) {
        FOE_LOG(foeResource, Fatal,
                "foeImageLoader being deinitialized with {} active uploads in progress!",
                mActiveUploads.load());
    }

    if (mGfxUploadContext != FOE_NULL_HANDLE)
        foeGfxDestroyUploadContext(mGfxUploadContext);

    mGfxSession = FOE_NULL_HANDLE;
}

bool foeImageLoader::initialized() const noexcept { return mGfxSession != FOE_NULL_HANDLE; }

void foeImageLoader::processLoadRequests() {
    mUploadingSync.lock();
    auto uploads = std::move(mUploadingData);
    mUploadingSync.unlock();

    for (auto &it : uploads) {
        processUpload(it.pImage, it.uploadRequest, it.uploadBuffer, it.data);
    }
}

void foeImageLoader::processUnloadRequests() {
    mUnloadSync.lock();
    ++mCurrentUnloadRequests;
    if (mCurrentUnloadRequests == &mUnloadRequestLists[mUnloadRequestLists.size()]) {
        mCurrentUnloadRequests = &mUnloadRequestLists[0];
    }
    auto unloadRequests = std::move(*mCurrentUnloadRequests);
    mUnloadSync.unlock();

    for (auto &data : unloadRequests) {
        // Nothing to do, compiles out
        VkDevice device = foeGfxVkGetDevice(mGfxSession);
        VmaAllocator allocator = foeGfxVkGetAllocator(mGfxSession);

        if (data.sampler != VK_NULL_HANDLE)
            vkDestroySampler(device, data.sampler, nullptr);

        if (data.view != VK_NULL_HANDLE)
            vkDestroyImageView(device, data.view, nullptr);

        if (data.image != VK_NULL_HANDLE)
            vmaDestroyImage(allocator, data.image, data.alloc);
    }
}

void foeImageLoader::requestResourceLoad(foeImage *pImage) {
    ++mActiveJobs;
    mAsyncJobs([this, pImage] {
        startUpload(pImage);
        --mActiveJobs;
    });
}

void foeImageLoader::requestResourceUnload(foeImage *pImage) {
    std::scoped_lock unloadLock{mUnloadSync};
    std::scoped_lock writeLock{pImage->dataWriteLock};

    // Only unload if it's 'loaded' and useCount is zero
    if (pImage->loadState == foeResourceLoadState::Loaded && pImage->getUseCount() == 0) {
        mCurrentUnloadRequests->emplace_back(std::move(pImage->data));

        pImage->data = {};
        pImage->loadState = foeResourceLoadState::Unloaded;
    }
}

#include <FreeImage.h>
#include <foe/graphics/vk/format.hpp>
#include <foe/graphics/vk/image.hpp>
#include <vk_error_code.hpp>

void foeImageLoader::startUpload(foeImage *pImage) {
    // First, try to enter the 'loading' state
    auto expected = pImage->loadState.load();
    while (expected != foeResourceLoadState::Loading) {
        if (pImage->loadState.compare_exchange_weak(expected, foeResourceLoadState::Loading))
            break;
    }
    if (expected == foeResourceLoadState::Loading) {
        FOE_LOG(foeResource, Warning, "Attempted to load foeImage {} in parrallel",
                static_cast<void *>(pImage))
        return;
    }

    // Start the actual loading process
    std::error_code errC;
    VkResult vkRes{VK_SUCCESS};

    foeImageCreateInfo createInfo;

    foeGfxUploadRequest gfxUploadRequest;
    foeGfxUploadBuffer gfxUploadBuffer;
    foeImage::Data imgData{};

    bool read = mImportFunction(pImage->getID(), createInfo);
    if (!read) {
        errC = FOE_RESOURCE_ERROR_IMPORT_FAILED;
        goto LOADING_FAILED;
    }

    { // Import the data
        // Determine the image format
        FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFileType(createInfo.fileName.c_str());
        if (imageFormat == FIF_UNKNOWN) {
            FreeImage_GetFIFFromFilename(createInfo.fileName.c_str());
        }
        if (imageFormat == FIF_UNKNOWN) {
            FOE_LOG(foeResource, Error, "Could not determine image format for: {}",
                    createInfo.fileName)
            errC = FOE_RESOURCE_ERROR_IMPORT_FAILED;
            goto LOADING_FAILED;
        }

        // Load the image into memory
        auto *bitmap = FreeImage_Load(imageFormat, createInfo.fileName.c_str(), 0);
        if (bitmap == nullptr) {
            FOE_LOG(foeResource, Error, "Failed to load image: {}", createInfo.fileName)
            errC = FOE_RESOURCE_ERROR_IMPORT_FAILED;
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
        auto bpp = bytesPerPixel(format, VK_IMAGE_ASPECT_COLOR_BIT);
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
                              bytesPerPixel(format, VK_IMAGE_ASPECT_COLOR_BIT);
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
                    mGfxUploadContext, &subresourceRange, copyRegions.size(), copyRegions.data(),
                    gfxUploadBuffer, imgData.image, VK_ACCESS_SHADER_READ_BIT,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &gfxUploadRequest);
                if (vkRes != VK_SUCCESS) {
                    goto LOADING_FAILED;
                }

                auto errC = foeSubmitUploadDataCommands(mGfxUploadContext, gfxUploadRequest);
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
        FOE_LOG(foeResource, Error, "Failed to load foeImage {} with error {}:{}",
                static_cast<void *>(pImage), errC.value(), errC.message())

        pImage->loadState = foeResourceLoadState::Failed;

        // No longer using the reference, decrement.
        pImage->decrementRefCount();

        if (gfxUploadRequest != FOE_NULL_HANDLE) {
            ++mActiveUploads;
            // A partial upload success, leave pImage as nullptr, so the upload completes then the
            // data is discarded
            processUpload(nullptr, gfxUploadRequest, gfxUploadBuffer, imgData);
        } else {
            // No target image, discard all the data
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
        // Stash the data in a list while we wait for the upload to complete, to later be processed
        // by processUploadRequest
        ++mActiveUploads;

        processUpload(pImage, gfxUploadRequest, gfxUploadBuffer, imgData);
    }
}

void foeImageLoader::processUpload(foeImage *pImage,
                                   foeGfxUploadRequest uploadRequest,
                                   foeGfxUploadBuffer uploadBuffer,
                                   foeImage::Data data) {
    auto requestStatus = foeGfxGetUploadRequestStatus(uploadRequest);
    if (requestStatus == FOE_GFX_UPLOAD_REQUEST_STATUS_COMPLETE) {
        if (uploadBuffer != FOE_NULL_HANDLE)
            foeGfxDestroyUploadBuffer(mGfxUploadContext, uploadBuffer);
        foeGfxDestroyUploadRequest(mGfxUploadContext, uploadRequest);

        // It's done, swap the data in
        if (pImage != nullptr) {
            std::scoped_lock writeLock{pImage->dataWriteLock};
            foeImage::Data oldData = std::move(pImage->data);
            pImage->data = std::move(data);
            pImage->loadState = foeResourceLoadState::Loaded;

            // If there was active old data that we just wrote over, send it to be unloaded
            {
                std::scoped_lock unloadLock{mUnloadSync};
                mCurrentUnloadRequests->emplace_back(std::move(oldData));
            }
        } else {
            // No target image, discard all the data
            VkDevice device = foeGfxVkGetDevice(mGfxSession);
            VmaAllocator allocator = foeGfxVkGetAllocator(mGfxSession);

            if (data.sampler != VK_NULL_HANDLE)
                vkDestroySampler(device, data.sampler, nullptr);
            if (data.view != VK_NULL_HANDLE)
                vkDestroyImageView(device, data.view, nullptr);
            if (data.image != VK_NULL_HANDLE)
                vmaDestroyImage(allocator, data.image, data.alloc);
        }

        --mActiveUploads;
    } else if (requestStatus != FOE_GFX_UPLOAD_REQUEST_STATUS_INCOMPLETE) {
        if (uploadBuffer != FOE_NULL_HANDLE)
            foeGfxDestroyUploadBuffer(mGfxUploadContext, uploadBuffer);
        foeGfxDestroyUploadRequest(mGfxUploadContext, uploadRequest);

        // There was an error, this is lost
        if (pImage != nullptr) {
            pImage->loadState = foeResourceLoadState::Failed;
        } else {
            // No target image, discard all the data
            VkDevice device = foeGfxVkGetDevice(mGfxSession);
            VmaAllocator allocator = foeGfxVkGetAllocator(mGfxSession);

            if (data.sampler != VK_NULL_HANDLE)
                vkDestroySampler(device, data.sampler, nullptr);
            if (data.view != VK_NULL_HANDLE)
                vkDestroyImageView(device, data.view, nullptr);
            if (data.image != VK_NULL_HANDLE)
                vmaDestroyImage(allocator, data.image, data.alloc);
        }

        --mActiveUploads;
    } else {
        // It's not yet complete, add to the uploading data list
        std::scoped_lock lock{mUploadingSync};

        mUploadingData.emplace_back(ImageUpload{
            .pImage = pImage,
            .uploadRequest = uploadRequest,
            .uploadBuffer = uploadBuffer,
            .data = data,
        });
    }
}