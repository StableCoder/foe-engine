// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "register.hpp"

#include <foe/graphics/resource/imgui/registration.hpp>
#include <foe/physics/imgui/registration.hpp>
#include <foe/position/imgui/registration.hpp>

#include "bringup_registration.hpp"

foeResultSet registerImGui(foeSimulationImGuiRegistrar *pRegistrar) noexcept {
    foeResultSet result;

    result = foePositionImGuiRegister(pRegistrar);
    if (result.value != FOE_SUCCESS)
        goto REGISTRATION_FAILED;

    result = foePhysicsImGuiRegister(pRegistrar);
    if (result.value != FOE_SUCCESS)
        goto REGISTRATION_FAILED;

    result = foeGraphicsResourceImGuiRegister(pRegistrar);
    if (result.value != FOE_SUCCESS)
        goto REGISTRATION_FAILED;

    result = foeBringupImGuiRegister(pRegistrar);
    if (result.value != FOE_SUCCESS)
        goto REGISTRATION_FAILED;

REGISTRATION_FAILED:
    if (result.value != FOE_SUCCESS)
        deregisterImGui(pRegistrar);

    return result;
}

void deregisterImGui(foeSimulationImGuiRegistrar *pRegistrar) noexcept {
    foeBringupImGuiDeregister(pRegistrar);
    foeGraphicsResourceImGuiDeregister(pRegistrar);
    foePhysicsImGuiDeregister(pRegistrar);
    foePositionImGuiDeregister(pRegistrar);
}