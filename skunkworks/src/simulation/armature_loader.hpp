// Copyright (C) 2021-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef ARMATURE_LOADER_HPP
#define ARMATURE_LOADER_HPP

#include <foe/resource/create_info.h>
#include <foe/resource/pool.h>
#include <foe/resource/resource.h>
#include <foe/result.h>
#include <foe/simulation/simulation.h>

#include "armature.hpp"

#include <filesystem>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

class foeArmatureLoader {
  public:
    foeResultSet initialize(foeResourcePool resourcePool,
                            void *pExternalFileSearchContext,
                            PFN_foeSimulationExternalFileSearch pfnExternalFileSearch);
    void deinitialize();
    bool initialized() const noexcept;

    void maintenance();

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
    void *pExternalFileSearchContext;
    PFN_foeSimulationExternalFileSearch pfnExternalFileSearch;

    struct LoadData {
        foeResource resource;
        PFN_foeResourcePostLoad postLoadFn;
        foeArmature data;
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
};

#endif // ARMATURE_LOADER_HPP