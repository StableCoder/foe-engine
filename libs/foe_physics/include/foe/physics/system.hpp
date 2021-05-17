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

#include <foe/data_pool.hpp>
#include <foe/ecs/id.hpp>
#include <foe/physics/export.h>
#include <foe/physics/rigid_body.hpp>

#include <memory>

class foePhysCollisionShapeLoader;
class foePhysCollisionShapePool;
class foePosition3dPool;

FOE_PHYSICS_EXPORT void initPhysics();

FOE_PHYSICS_EXPORT void processPhysics(foePhysCollisionShapeLoader &collisionShapeLoader,
                                       foePhysCollisionShapePool &collisionShapePool,
                                       foeDataPool<foeEntityID, foePhysRigidBody> &rigidBodyPool,
                                       foePosition3dPool &positionPool,
                                       float timePassed);

#endif // FOE_PHYSICS_SYSTEM_HPP