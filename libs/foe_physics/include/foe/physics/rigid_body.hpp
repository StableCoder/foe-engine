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

#ifndef FOE_PHYSICS_RIGID_BODY_HPP
#define FOE_PHYSICS_RIGID_BODY_HPP

#include <btBulletDynamicsCommon.h>
#include <foe/ecs/id.hpp>

#include <memory>

struct foePhysRigidBody {
    std::unique_ptr<btRigidBody> rigidBody;
    float mass;
    foeResourceID collisionShape;
};

#endif // FOE_PHYSICS_RIGID_BODY_HPP