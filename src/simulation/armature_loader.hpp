// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_RESOURCE_ARMATURE_LOADER_HPP
#define FOE_RESOURCE_ARMATURE_LOADER_HPP

#include "armature.hpp"
#include <foe/managed_memory.h>
#include <foe/resource/pool.h>
#include <foe/resource/resource.h>
#include <foe/result.h>

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
    std::function<foeResultSet(char const *, foeManagedMemory *)> mExternalFileSearchFn;

    struct LoadData {
        foeResource resource;
        foeResourceCreateInfo createInfo;
        PFN_foeResourcePostLoad *pPostLoadFn;
        foeArmature data;
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
};

#endif // FOE_RESOURCE_ARMATURE_LOADER_HPP