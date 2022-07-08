// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_PHYSICS_IMGUI_REGISTRATION_HPP
#define FOE_PHYSICS_IMGUI_REGISTRATION_HPP

#include <foe/error_code.h>
#include <foe/physics/imgui/export.h>

class foeSimulationImGuiRegistrar;

FOE_PHYSICS_IMGUI_EXPORT foeResult foePhysicsImGuiRegister(foeSimulationImGuiRegistrar *pRegistrar);

FOE_PHYSICS_IMGUI_EXPORT foeResult
foePhysicsImGuiDeregister(foeSimulationImGuiRegistrar *pRegistrar);

#endif // FOE_PHYSICS_IMGUI_REGISTRATION_HPP