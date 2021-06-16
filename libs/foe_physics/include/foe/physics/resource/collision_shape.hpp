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

#ifndef FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_HPP
#define FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_HPP

#include <btBulletDynamicsCommon.h>
#include <foe/ecs/id.hpp>
#include <foe/physics/export.h>
#include <foe/resource/create_info_base.hpp>
#include <foe/resource/load_state.hpp>
#include <glm/glm.hpp>

#include <atomic>
#include <memory>
#include <mutex>

class foePhysCollisionShapeLoader;

struct foePhysCollisionShapeCreateInfo : public foeResourceCreateInfoBase {
    glm::vec3 boxSize;
};

struct FOE_PHYSICS_EXPORT foePhysCollisionShape {
    friend foePhysCollisionShapeLoader;

    foePhysCollisionShape(foeResourceID id,
                          void (*pLoadFn)(void *, void *, bool),
                          void *pLoadContext);
    ~foePhysCollisionShape();

    foeId getID() const noexcept;
    foeResourceLoadState getLoadState() const noexcept;

    int incrementRefCount() noexcept;
    int decrementRefCount() noexcept;
    int getRefCount() const noexcept;

    int incrementUseCount() noexcept;
    int decrementUseCount() noexcept;
    int getUseCount() const noexcept;

    void requestLoad();
    void requestUnload();

    // General
    foeId id;
    std::atomic<foeResourceLoadState> loadState{foeResourceLoadState::Unloaded};
    std::atomic_int refCount{0};
    std::atomic_int useCount{0};

    // Specialization
    void (*mpLoadFn)(void *, void *, bool);
    void *mpLoadContext;

    std::mutex dataWriteLock{};
    std::unique_ptr<foePhysCollisionShapeCreateInfo> createInfo{nullptr};
    struct Data {
        std::unique_ptr<btCollisionShape> collisionShape;
    } data;
};

#endif // FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_HPP