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

#ifndef STATE_POOLS_HPP
#define STATE_POOLS_HPP

#include <foe/data_pool.hpp>
#include <foe/ecs/id.hpp>
#include <foe/physics/rigid_body.hpp>
#include <foe/position/3d.hpp>

#include <map>
#include <memory>

#include "armature_state.hpp"
#include "render_state.hpp"

struct StatePools {
    std::map<foeId, foeRenderState> renderStates;
    std::map<foeId, foeArmatureState> armatureStates;

    foeDataPool<foeEntityID, std::unique_ptr<Position3D>> position;
    foeDataPool<foeEntityID, foePhysRigidBody> rigidBody;

    void maintenance() {
        position.maintenance();
        rigidBody.maintenance();
    }
};

#endif // STATE_POOLS_HPP