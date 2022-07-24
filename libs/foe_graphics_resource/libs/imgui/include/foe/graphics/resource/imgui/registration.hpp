// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRPAHICS_RESOURCE_IMGUI_REGISTRATION_HPP
#define FOE_GRPAHICS_RESOURCE_IMGUI_REGISTRATION_HPP

#include <foe/error_code.h>
#include <foe/graphics/resource/imgui/export.h>

class foeSimulationImGuiRegistrar;

FOE_GFX_RES_IMGUI_EXPORT foeResultSet
foeGraphicsResourceImGuiRegister(foeSimulationImGuiRegistrar *pRegistrar);

FOE_GFX_RES_IMGUI_EXPORT foeResultSet
foeGraphicsResourceImGuiDeregister(foeSimulationImGuiRegistrar *pRegistrar);

#endif // FOE_GRPAHICS_RESOURCE_IMGUI_REGISTRATION_HPP