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

#ifndef FOE_GRAPHICS_RESOURCE_IMAGE_LOADER_HPP
#define FOE_GRAPHICS_RESOURCE_IMAGE_LOADER_HPP

#include <foe/graphics/resource/export.h>
#include <foe/graphics/resource/image.hpp>
#include <foe/graphics/session.hpp>
#include <foe/graphics/type_defs.hpp>
#include <foe/graphics/upload_buffer.hpp>
#include <foe/graphics/upload_context.hpp>
#include <foe/graphics/upload_request.hpp>
#include <foe/simulation/core/loader.hpp>

#include <array>
#include <filesystem>
#include <functional>
#include <vector>

struct foeImageCreateInfo : public foeResourceCreateInfoBase {
    std::string fileName;
};

class FOE_GFX_RES_EXPORT foeImageLoader : public foeResourceLoaderBase {
  public:
    std::error_code initialize(
        foeGfxSession session,
        std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn);
    void deinitialize();
    bool initialized() const noexcept;

    void gfxMaintenance();

    bool canProcessCreateInfo(foeResourceCreateInfoBase *pCreateInfo) final;
    void load(void *pResource,
              std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
              void (*pPostLoadFn)(void *, std::error_code)) final;

  private:
    static void unloadResource(void *pContext,
                               void *pResource,
                               uint32_t resourceIteration,
                               bool immediateUnload);

    foeGfxSession mGfxSession{FOE_NULL_HANDLE};
    std::function<std::filesystem::path(std::filesystem::path)> mExternalFileSearchFn;

    foeGfxUploadContext mGfxUploadContext{FOE_NULL_HANDLE};

    struct LoadData {
        foeImage *pImage;
        void (*pPostLoadFn)(void *, std::error_code);
        foeImage::Data data;

        foeGfxUploadRequest uploadRequest{FOE_NULL_HANDLE};
        foeGfxUploadBuffer uploadBuffer{FOE_NULL_HANDLE};
    };

    std::mutex mLoadSync;
    std::vector<LoadData> mToLoad;

    struct UnloadData {
        foeImage *pImage;
        uint32_t iteration;
    };

    std::mutex mUnloadRequestsSync;
    std::vector<UnloadData> mUnloadRequests;

    size_t mDataDestroyIndex{0};
    std::array<std::vector<foeImage::Data>, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1>
        mDataDestroyLists{};
};

#endif // FOE_GRAPHICS_RESOURCE_IMAGE_LOADER_HPP