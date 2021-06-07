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

#ifndef FOE_PHYSICS_SYSTEM_HPP
#define FOE_PHYSICS_SYSTEM_HPP

#include <btBulletDynamicsCommon.h>
#include <foe/ecs/id.hpp>
#include <foe/physics/export.h>
#include <foe/simulation/system_base.hpp>

#include <memory>
#include <vector>

class foePhysCollisionShapeLoader;
class foePhysCollisionShapePool;
class foeRigidBodyPool;
class foePosition3dPool;

struct foeRigidBody;
struct foePosition3d;
struct foePhysCollisionShape;

class FOE_PHYSICS_EXPORT foePhysicsSystem : public foeSystemBase {
  public:
    foePhysicsSystem();
    ~foePhysicsSystem();

    void initialize(foePhysCollisionShapeLoader *pCollisionShapeLoader,
                    foePhysCollisionShapePool *pCollisionShapePool,
                    foeRigidBodyPool *pRigidBodyPool,
                    foePosition3dPool *pPosition3dPool);
    void deinitialize();

    void process(float timePassed);

  private:
    void addObject(foeEntityID entity,
                   foeRigidBody *pRigidBody,
                   foePosition3d *pPosition3d,
                   foePhysCollisionShape *pCollisionShape);
    void removeObject(foeEntityID entity, foeRigidBody *pRigidBody);

    // Resources
    foePhysCollisionShapeLoader *mpCollisionShapeLoader;
    foePhysCollisionShapePool *mpCollisionShapePool;

    // Components
    foeRigidBodyPool *mpRigidBodyPool;
    foePosition3dPool *mpPosition3dPool;

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