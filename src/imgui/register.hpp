// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IMGUI_REGISTER_ELEMENTS_HPP
#define IMGUI_REGISTER_ELEMENTS_HPP

#include <foe/error_code.h>

class foeSimulationImGuiRegistrar;

foeResult registerImGui(foeSimulationImGuiRegistrar *pRegistrar) noexcept;

void deregisterImGui(foeSimulationImGuiRegistrar *pRegistrar) noexcept;

#endif // IMGUI_REGISTER_ELEMENTS_HPP