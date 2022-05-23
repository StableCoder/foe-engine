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

#ifndef FOE_GRPAHICS_RESOURCE_IMGUI_REGISTRATION_HPP
#define FOE_GRPAHICS_RESOURCE_IMGUI_REGISTRATION_HPP

#include <foe/error_code.h>
#include <foe/graphics/resource/imgui/export.h>

class foeSimulationImGuiRegistrar;

FOE_GFX_RES_IMGUI_EXPORT foeResult
foeGraphicsResourceImGuiRegister(foeSimulationImGuiRegistrar *pRegistrar);

FOE_GFX_RES_IMGUI_EXPORT foeResult
foeGraphicsResourceImGuiDeregister(foeSimulationImGuiRegistrar *pRegistrar);

#endif // FOE_GRPAHICS_RESOURCE_IMGUI_REGISTRATION_HPP