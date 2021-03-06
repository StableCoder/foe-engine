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

#ifndef FOE_PHYSICS_COMPONENT_RIGID_BODY_POOL_HPP
#define FOE_PHYSICS_COMPONENT_RIGID_BODY_POOL_HPP

#include <foe/data_pool.hpp>
#include <foe/ecs/id.hpp>
#include <foe/imex/component_pool_base.hpp>
#include <foe/physics/component/rigid_body.hpp>

class foeRigidBodyPool : public foeComponentPoolBase,
                         public foeDataPool<foeEntityID, foeRigidBody> {
  public:
    void maintenance() override { foeDataPool<foeEntityID, foeRigidBody>::maintenance(); }
};

#endif // FOE_PHYSICS_COMPONENT_RIGID_BODY_POOL_HPP