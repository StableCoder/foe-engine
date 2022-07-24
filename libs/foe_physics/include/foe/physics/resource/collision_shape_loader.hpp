// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_LOADER_HPP
#define FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_LOADER_HPP

#include <foe/ecs/id.h>
#include <foe/physics/export.h>
#include <foe/physics/resource/collision_shape.hpp>
#include <foe/resource/pool.h>
#include <foe/resource/resource.h>

#include <mutex>
#include <vector>

class FOE_PHYSICS_EXPORT foeCollisionShapeLoader {
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

    struct LoadData {
        foeResource resource;
        foeResourceCreateInfo createInfo;
        PFN_foeResourcePostLoad *pPostLoadFn;
        foeCollisionShape data;
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

#endif // FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_LOADER_HPP