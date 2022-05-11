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

#ifndef FOE_PHYSICS_SYSTEM_HPP
#define FOE_PHYSICS_SYSTEM_HPP

#include <btBulletDynamicsCommon.h>
#include <foe/ecs/id.h>
#include <foe/physics/export.h>
#include <foe/resource/pool.h>
#include <foe/resource/resource.h>

#include <memory>
#include <system_error>
#include <vector>

class foeCollisionShapeLoader;
class foeRigidBodyPool;
class foePosition3dPool;

struct foeRigidBody;
struct foePosition3d;
struct foeCollisionShape;

class FOE_PHYSICS_EXPORT foePhysicsSystem {
  public:
    foePhysicsSystem();
    ~foePhysicsSystem();

    auto initialize(foeResourcePool resourcePool,
                    foeCollisionShapeLoader *pCollisionShapeLoader,
                    foeRigidBodyPool *pRigidBodyPool,
                    foePosition3dPool *pPosition3dPool) -> std::error_code;
    void deinitialize();
    bool initialized() const noexcept;

    void process(float timePassed);

  private:
    void addObject(foeEntityID entity,
                   foeRigidBody *pRigidBody,
                   foePosition3d *pPosition3d,
                   foeResource collisionShape);
    void removeObject(foeEntityID entity, foeRigidBody *pRigidBody);

    // Resources
    foeResourcePool mResourcePool{FOE_NULL_HANDLE};
    foeCollisionShapeLoader *mpCollisionShapeLoader{nullptr};

    // Components
    foeRigidBodyPool *mpRigidBodyPool{nullptr};
    foePosition3dPool *mpPosition3dPool{nullptr};

    // Physics World Instance Items
    std::unique_ptr<btBroadphaseInterface> mpBroadphase;
    std::unique_ptr<btDefaultCollisionConfiguration> mpCollisionConfig;
    std::unique_ptr<btCollisionDispatcher> mpCollisionDispatcher;
    std::unique_ptr<btSequentialImpulseConstraintSolver> mpSolver;
    std::unique_ptr<btDiscreteDynamicsWorld> mpWorld;

    // Lists entities that need some resources to load before being added to a world
    std::vector<foeEntityID> mAwaitingLoadingResources;
};

#endif // FOE_PHYSICS_SYSTEM_HPP