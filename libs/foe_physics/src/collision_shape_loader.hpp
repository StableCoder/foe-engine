// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef COLLISION_SHAPE_LOADER_HPP
#define COLLISION_SHAPE_LOADER_HPP

#include <foe/ecs/id.h>
#include <foe/physics/resource/collision_shape.hpp>
#include <foe/resource/create_info.h>
#include <foe/resource/pool.h>
#include <foe/resource/resource.h>

#include <mutex>
#include <vector>

class foeCollisionShapeLoader {
  public:
    ~foeCollisionShapeLoader();

    foeResultSet initialize(foeResourcePool resourcePool);
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

    struct LoadData {
        foeResource resource;
        PFN_foeResourcePostLoad postLoadFn;
        foeCollisionShape data;
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

#endif // COLLISION_SHAPE_LOADER_HPP