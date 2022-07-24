// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_POSITION_IMGUI_REGISTRATION_HPP
#define FOE_POSITION_IMGUI_REGISTRATION_HPP

#include <foe/position/imgui/export.h>
#include <foe/result.h>

class foeSimulationImGuiRegistrar;

FOE_POSITION_IMGUI_EXPORT foeResultSet
foePositionImGuiRegister(foeSimulationImGuiRegistrar *pRegistrar);

FOE_POSITION_IMGUI_EXPORT foeResultSet
foePositionImGuiDeregister(foeSimulationImGuiRegistrar *pRegistrar);

#endif // FOE_POSITION_IMGUI_REGISTRATION_HPP