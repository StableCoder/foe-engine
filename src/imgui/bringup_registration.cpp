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

#include "bringup_registration.hpp"

#include <foe/simulation/imgui/registrar.hpp>

#include "../armature_state_pool.hpp"
#include "../camera_pool.hpp"
#include "../render_state_pool.hpp"
#include "armature_state.hpp"
#include "camera.hpp"
#include "render_state.hpp"

namespace {

void imgui_foeBringupComponents(foeEntityID entity,
                                foeComponentPoolBase **ppPools,
                                size_t poolCount) {
    for (size_t i = 0; i < poolCount; ++i) {
        { // foeArmatureState
            auto *pPool = dynamic_cast<foeArmatureStatePool *>(ppPools[i]);
            if (pPool) {
                auto offset = pPool->find(entity);
                if (offset != pPool->size()) {
                    auto *pComponent = pPool->begin<1>() + offset;
                    imgui_foeArmatureState(pComponent);
                }
            }
        }

        { // Camera
            auto *pPool = dynamic_cast<foeCameraPool *>(ppPools[i]);
            if (pPool) {
                auto offset = pPool->find(entity);
                if (offset != pPool->size()) {
                    auto *pComponent = pPool->begin<1>() + offset;
                    auto *pCamera = dynamic_cast<Camera *>(pComponent->get());

                    if (pCamera)
                        imgui_Camera(pCamera);
                }
            }
        }

        { // foeRenderState
            auto *pPool = dynamic_cast<foeRenderStatePool *>(ppPools[i]);
            if (pPool) {
                auto offset = pPool->find(entity);
                if (offset != pPool->size()) {
                    auto *pComponent = pPool->begin<1>() + offset;
                    imgui_foeRenderState(pComponent);
                }
            }
        }
    }
}

} // namespace

auto foeBringupImGuiRegister(foeSimulationImGuiRegistrar *pRegistrar) -> std::error_code {
    return pRegistrar->registerElements(&imgui_foeBringupComponents, nullptr, nullptr);
}

auto foeBringupImGuiDeregister(foeSimulationImGuiRegistrar *pRegistrar) -> std::error_code {
    return pRegistrar->deregisterElements(&imgui_foeBringupComponents, nullptr, nullptr);
}