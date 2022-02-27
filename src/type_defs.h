/*
    Copyright (C) 2022 George Cave.

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

#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <foe/simulation/type_defs.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FOE_BRINGUP_APP_FUNCTIONALITY_ID FOE_SIMULATION_FUNCTIONALITY_ID(0)

typedef enum foeBringupStructureType {
    FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM,
    FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM,
    FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL,
    FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL,
} foeBringupStructureType;

#ifdef __cplusplus
}
#endif

#endif // TYPE_DEFS_H