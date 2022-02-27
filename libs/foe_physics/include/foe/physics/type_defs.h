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

#ifndef FOE_PHYSICS_TYPE_DEFS_H
#define FOE_PHYSICS_TYPE_DEFS_H

#include <foe/simulation/type_defs.h>

#define FOE_PHYSICS_FUNCTIONALITY_ID FOE_SIMULATION_FUNCTIONALITY_ID(2)

typedef enum foePhysicsStructureType {
    // Collision Shape
    FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE = FOE_PHYSICS_FUNCTIONALITY_ID,
    FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_POOL,
    FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER,
    // Systems
    FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM,
} foePhysicsStructureType;

#endif // FOE_PHYSICS_TYPE_DEFS_H