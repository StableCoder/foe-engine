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

#ifndef FOE_RESOURCE_IMAGE_LOADER_HPP
#define FOE_RESOURCE_IMAGE_LOADER_HPP

#include <foe/graphics/session.hpp>
#include <foe/graphics/type_defs.hpp>
#include <foe/graphics/upload_context.hpp>
#include <foe/resource/export.h>
#include <foe/resource/image.hpp>

#include <array>
#include <atomic>
#include <functional>
#include <mutex>
#include <system_error>
#include <vector>

class foeImageLoader {
  public:
    FOE_RES_EXPORT ~foeImageLoader();

    FOE_RES_EXPORT std::error_code initialize(
        foeGfxSession session,
        std::function<foeResourceCreateInfoBase *(foeId)> importFunction,
        std::function<void(std::function<void()>)> asynchronousJobs);
    FOE_RES_EXPORT void deinitialize();
    FOE_RES_EXPORT bool initialized() const noexcept;

    FOE_RES_EXPORT void processLoadRequests();
    FOE_RES_EXPORT void processUnloadRequests();

    FOE_RES_EXPORT void requestResourceLoad(foeImage *pImage);
    FOE_RES_EXPORT void requestResourceUnload(foeImage *pImage);

  private:
    struct ImageUpload {
        foeImage *pImage;
        foeGfxUploadRequest uploadRequest{FOE_NULL_HANDLE};
        foeGfxUploadBuffer uploadBuffer{FOE_NULL_HANDLE};
        foeImage::Data data{};
    };

    void startUpload(foeImage *pImage);

    // If pImage is nullptr, then once the upload is complete, the data is just discarded
    void processUpload(foeImage *pImage,
                       foeGfxUploadRequest uploadRequest,
                       foeGfxUploadBuffer uploadBuffer,
                       foeImage::Data data);

    foeGfxSession mGfxSession{FOE_NULL_HANDLE};
    foeGfxUploadContext mGfxUploadContext{FOE_NULL_HANDLE};

    std::function<foeResourceCreateInfoBase *(foeId)> mImportFunction;
    std::function<void(std::function<void()>)> mAsyncJobs;
    std::atomic_int mActiveJobs;

    std::mutex mUploadingSync{};
    std::vector<ImageUpload> mUploadingData;
    std::atomic_int mActiveUploads;

    std::mutex mUnloadSync{};
    std::array<std::vector<foeImage::Data>, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1>
        mUnloadRequestLists{};
    std::array<std::vector<foeImage::Data>, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1>::iterator
        mCurrentUnloadRequests{mUnloadRequestLists.begin()};
};

#endif // FOE_RESOURCE_IMAGE_LOADER_HPP