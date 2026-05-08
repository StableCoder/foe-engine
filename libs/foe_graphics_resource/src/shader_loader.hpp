// Copyright (C) 2021-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef SHADER_LOADER_HPP
#define SHADER_LOADER_HPP

#include <foe/graphics/resource/shader.hpp>
#include <foe/graphics/type_defs.h>
#include <foe/graphics/vk/shader.h>
#include <foe/resource/create_info.h>
#include <foe/resource/pool.h>
#include <foe/resource/resource.h>
#include <foe/simulation/simulation.hpp>

#include <array>
#include <mutex>
#include <vector>

class foeShaderLoader {
  public:
    foeResultSet initialize(foeResourcePool resourcePool,
                            void *pExternalFileSearchContext,
                            PFN_foeSimulationExternalFileSearch pfnExternalFileSearch);
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
                     PFN_foeResourcePostLoad postLoadFn);

  private:
    static void unloadResource(void *pContext,
                               foeResource resource,
                               uint32_t resourceIteration,
                               PFN_foeResourceUnloadCall unloadCallFn,
                               bool immediateUnload);

    void load(foeResource resource,
              foeResourceCreateInfo createInfo,
              PFN_foeResourcePostLoad postLoadFn);

    foeResourcePool mResourcePool{FOE_NULL_HANDLE};
    void *pExternalFileSearchContext = nullptr;
    PFN_foeSimulationExternalFileSearch pfnExternalFileSearch = nullptr;

    foeGfxSession mGfxSession{FOE_NULL_HANDLE};

    struct LoadData {
        foeResource resource;
        PFN_foeResourcePostLoad postLoadFn;
        foeShader data;
    };

    std::mutex mLoadSync;
    std::vector<LoadData> mLoadRequests;

    struct UnloadData {
        foeResource resource;
        uint32_t iteration;
        PFN_foeResourceUnloadCall unloadCallFn;
    };

    std::mutex mUnloadSync;
    std::vector<UnloadData> mUnloadRequests;

    std::mutex mDestroySync;
    size_t mDataDestroyIndex{0};
    std::array<std::vector<foeShader>, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1> mDataDestroyLists{};
};

#endif // SHADER_LOADER_HPP