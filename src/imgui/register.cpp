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

#include "register.hpp"

#include <foe/physics/imgui/registration.hpp>
#include <foe/position/imgui/registration.hpp>

#include "bringup_registration.hpp"

auto registerImGui(foeSimulationImGuiRegistrar *pRegistrar) noexcept -> std::error_code {
    std::error_code errC;

    errC = foePositionImGuiRegister(pRegistrar);
    if (errC)
        goto REGISTRATION_FAILED;

    errC = foePhysicsImGuiRegister(pRegistrar);
    if (errC)
        goto REGISTRATION_FAILED;

    errC = foeBringupImGuiRegister(pRegistrar);
    if (errC)
        goto REGISTRATION_FAILED;

REGISTRATION_FAILED:
    if (errC)
        deregisterImGui(pRegistrar);

    return errC;
}

void deregisterImGui(foeSimulationImGuiRegistrar *pRegistrar) noexcept {
    foeBringupImGuiDeregister(pRegistrar);
    foePhysicsImGuiDeregister(pRegistrar);
    foePositionImGuiDeregister(pRegistrar);
}