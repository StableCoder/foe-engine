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

#include <foe/physics/component/rigid_body_pool.hpp>
#include <foe/position/component/3d_pool.hpp>

#include "armature_state.hpp"
#include "camera_pool.hpp"
#include "render_state.hpp"

struct StatePools {
    foeArmatureStatePool armatureState;
    foeRenderStatePool renderState;

    foePosition3dPool position;
    foeCameraPool camera;
    foeRigidBodyPool rigidBody;

    void maintenance() {
        armatureState.maintenance();
        renderState.maintenance();

        position.maintenance();
        camera.maintenance();
        rigidBody.maintenance();
    }
};

#endif // STATE_POOLS_HPP