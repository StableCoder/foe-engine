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

#ifndef FOE_GRAPHICS_RESOURCE_IMAGE_LOADER_HPP
#define FOE_GRAPHICS_RESOURCE_IMAGE_LOADER_HPP

#include <foe/graphics/resource/export.h>
#include <foe/graphics/resource/image.hpp>
#include <foe/graphics/session.h>
#include <foe/graphics/type_defs.hpp>
#include <foe/graphics/upload_buffer.hpp>
#include <foe/graphics/upload_context.hpp>
#include <foe/graphics/upload_request.hpp>
#include <foe/resource/pool.h>
#include <foe/resource/resource.h>

#include <array>
#include <filesystem>
#include <functional>
#include <mutex>
#include <vector>

class FOE_GFX_RES_EXPORT foeImageLoader {
  public:
    foeResult initialize(
        foeResourcePool resourcePool,
        std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn);
    void deinitialize();
    bool initialized() const noexcept;

    foeResult initializeGraphics(foeGfxSession gfxSession);
    void deinitializeGraphics();
    bool initializedGraphics() const noexcept;

    void gfxMaintenance();

    static bool canProcessCreateInfo(foeResourceCreateInfo createInfo);
    static void load(void *pLoader,
                     foeResource resource,
                     foeResourceCreateInfo createInfo,
                     PFN_foeResourcePostLoad *pPostLoadFn);

  private:
    static void unloadResource(void *pContext,
                               foeResource resource,
                               uint32_t resourceIteration,
                               PFN_foeResourceUnloadCall *pUnloadCallFn,
                               bool immediateUnload);

    void load(foeResource resource,
              foeResourceCreateInfo createInfo,
              PFN_foeResourcePostLoad *pPostLoadFn);

    foeResourcePool mResourcePool{FOE_NULL_HANDLE};
    std::function<std::filesystem::path(std::filesystem::path)> mExternalFileSearchFn;

    foeGfxSession mGfxSession{FOE_NULL_HANDLE};

    foeGfxUploadContext mGfxUploadContext{FOE_NULL_HANDLE};

    struct LoadData {
        foeResource resource;
        foeResourceCreateInfo createInfo;
        PFN_foeResourcePostLoad *pPostLoadFn;
        foeImage data;

        foeGfxUploadRequest uploadRequest{FOE_NULL_HANDLE};
        foeGfxUploadBuffer uploadBuffer{FOE_NULL_HANDLE};
    };

    std::mutex mLoadSync;
    std::vector<LoadData> mLoadRequests;

    struct UnloadData {
        foeResource resource;
        uint32_t iteration;
        PFN_foeResourceUnloadCall *pUnloadCallFn;
    };

    std::mutex mUnloadSync;
    std::vector<UnloadData> mUnloadRequests;

    std::mutex mDestroySync;
    size_t mDataDestroyIndex{0};
    std::array<std::vector<foeImage>, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1> mDataDestroyLists{};
};

#endif // FOE_GRAPHICS_RESOURCE_IMAGE_LOADER_HPP