// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_SHADER_LOADER_HPP
#define FOE_GRAPHICS_RESOURCE_SHADER_LOADER_HPP

#include <foe/graphics/resource/export.h>
#include <foe/graphics/resource/shader.hpp>
#include <foe/graphics/type_defs.h>
#include <foe/graphics/vk/shader.hpp>
#include <foe/resource/pool.h>
#include <foe/resource/resource.h>

#include <array>
#include <filesystem>
#include <functional>
#include <mutex>
#include <vector>

class FOE_GFX_RES_EXPORT foeShaderLoader {
  public:
    foeResultSet initialize(
        foeResourcePool resourcePool,
        std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn);
    void deinitialize();
    bool initialized() const noexcept;

    foeResultSet initializeGraphics(foeGfxSession gfxSession);
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

    struct LoadData {
        foeResource resource;
        foeResourceCreateInfo createInfo;
        PFN_foeResourcePostLoad *pPostLoadFn;
        foeShader data;
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
    std::array<std::vector<foeShader>, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1> mDataDestroyLists{};
};

#endif // FOE_GRAPHICS_RESOURCE_SHADER_LOADER_HPP