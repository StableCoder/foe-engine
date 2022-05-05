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

#ifndef FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_POOL_HPP
#define FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_POOL_HPP

#include <foe/ecs/id.h>
#include <foe/physics/export.h>
#include <foe/resource/resource.h>
#include <foe/resource/resource_fns.h>

#include <functional>
#include <shared_mutex>
#include <vector>

struct foeCollisionShape;

class FOE_PHYSICS_EXPORT foeCollisionShapePool {
  public:
    foeCollisionShapePool(foeResourceFns const &resourceFns);
    ~foeCollisionShapePool();

    foeResource add(foeResourceID resource);
    foeResource findOrAdd(foeResourceID resource);
    foeResource find(foeResourceID resource);

    void setAsyncTaskFn(PFN_foeScheduleTask scheduleAsyncTask, void *pScheduleAsyncTaskContext);

    void unloadAll();

  private:
    FOE_PHYSICS_NO_EXPORT foeResourceFns mResourceFns;
    FOE_PHYSICS_NO_EXPORT std::shared_mutex mSync;
    FOE_PHYSICS_NO_EXPORT std::vector<foeResource> mResources;
};

#endif // FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_POOL_HPP