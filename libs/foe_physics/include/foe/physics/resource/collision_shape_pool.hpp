/*
    Copyright (C) 2021 George Cave.

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

#include <foe/ecs/id.hpp>
#include <foe/physics/export.h>
#include <foe/resource/pool_base.hpp>
#include <foe/simulation/resource_fns.hpp>

#include <functional>
#include <shared_mutex>
#include <vector>

struct foeCollisionShape;
class foeCollisionShapeLoader;
struct foeResourceCreateInfoBase;

class FOE_PHYSICS_EXPORT foeCollisionShapePool : public foeResourcePoolBase {
  public:
    foeCollisionShapePool(foeResourceFns const &resourceFns);
    ~foeCollisionShapePool();

    foeCollisionShape *add(foeResourceID resource);
    foeCollisionShape *findOrAdd(foeResourceID resource);
    foeCollisionShape *find(foeResourceID id);

    void unloadAll();

    auto getDataVector() { return mCollisionShapes; }

  private:
    FOE_PHYSICS_NO_EXPORT foeResourceFns const mResourceFns;
    FOE_PHYSICS_NO_EXPORT std::shared_mutex mSync;
    FOE_PHYSICS_NO_EXPORT std::vector<foeCollisionShape *> mCollisionShapes;
};

#endif // FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_POOL_HPP