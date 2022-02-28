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

#include <foe/position/imgui/registration.hpp>

#include <foe/position/component/3d_pool.hpp>
#include <foe/simulation/imgui/registrar.hpp>

#include "position_3d.hpp"

namespace {

void imgui_foePositionComponents(foeEntityID entity, foeSimulationState const *pSimulationState) {
    // foePosition3d
    auto *pPool = (foePosition3dPool *)foeSimulationGetComponentPool(
        pSimulationState, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);
    if (pPool != nullptr) {
        auto offset = pPool->find(entity);
        if (offset != pPool->size()) {
            auto *pComponent = pPool->begin<1>() + offset;

            imgui_foePosition3d(pComponent->get());
        }
    }
}

} // namespace

auto foePositionImGuiRegister(foeSimulationImGuiRegistrar *pRegistrar) -> std::error_code {
    return pRegistrar->registerElements(&imgui_foePositionComponents, nullptr, nullptr);
}

auto foePositionImGuiDeregister(foeSimulationImGuiRegistrar *pRegistrar) -> std::error_code {
    return pRegistrar->deregisterElements(&imgui_foePositionComponents, nullptr, nullptr);
}