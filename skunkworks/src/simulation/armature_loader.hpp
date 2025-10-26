// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef ARMATURE_LOADER_HPP
#define ARMATURE_LOADER_HPP

#include <foe/managed_memory.h>
#include <foe/resource/create_info.h>
#include <foe/resource/pool.h>
#include <foe/resource/resource.h>
#include <foe/result.h>

#include "armature.hpp"

#include <filesystem>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

class foeArmatureLoader {
  public:
    foeResultSet initialize(
        foeResourcePool resourcePool,
        std::function<foeResultSet(char const *, foeManagedMemory *)> externalFileSearchFn);
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
    std::function<foeResultSet(char const *, foeManagedMemory *)> mExternalFileSearchFn;

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