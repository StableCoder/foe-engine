// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IMGUI_BRINGUP_REGISTRATION_HPP
#define IMGUI_BRINGUP_REGISTRATION_HPP

#include <foe/result.h>

class foeSimulationImGuiRegistrar;

foeResultSet foeBringupImGuiRegister(foeSimulationImGuiRegistrar *pRegistrar);

foeResultSet foeBringupImGuiDeregister(foeSimulationImGuiRegistrar *pRegistrar);

#endif // IMGUI_BRINGUP_REGISTRATION_HPP