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

#ifndef FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_LOADER_HPP
#define FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_LOADER_HPP

#include <foe/ecs/id.h>
#include <foe/physics/export.h>
#include <foe/physics/resource/collision_shape.hpp>
#include <foe/resource/pool.h>
#include <foe/resource/resource.h>

#include <mutex>
#include <system_error>
#include <vector>

class FOE_PHYSICS_EXPORT foeCollisionShapeLoader {
  public:
    ~foeCollisionShapeLoader();

    std::error_code initialize(foeResourcePool resourcePool);
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
    std::vector<LoadData> mToLoad;

    struct UnloadData {
        foeResource resource;
        uint32_t iteration;
        PFN_foeResourceUnloadCall *pUnloadCallFn;
    };

    std::mutex mUnloadRequestsSync;
    std::vector<UnloadData> mUnloadRequests;
};

#endif // FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_LOADER_HPP