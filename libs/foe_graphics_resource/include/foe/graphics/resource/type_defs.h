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

#ifndef FOE_GRAPHICS_RESOURCE_TYPE_DEFS_H
#define FOE_GRAPHICS_RESOURCE_TYPE_DEFS_H

#include <foe/simulation/type_defs.h>

#define FOE_GRAPHICS_RESOURCE_FUNCTIONALITY_ID FOE_SIMULATION_FUNCTIONALITY_ID(1)

enum foeGraphicsResourceStructureType {
    // Image
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE = FOE_GRAPHICS_RESOURCE_FUNCTIONALITY_ID,
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_POOL,
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER,
    // Material
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL,
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_POOL,
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER,
    // Mesh
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH,
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_POOL,
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER,
    // Shader
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER,
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_POOL,
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER,
    // VertexDescriptor
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR,
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_POOL,
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER,
};

#endif // FOE_GRAPHICS_RESOURCE_TYPE_DEFS_H