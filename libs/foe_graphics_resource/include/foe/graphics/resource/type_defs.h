// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_TYPE_DEFS_H
#define FOE_GRAPHICS_RESOURCE_TYPE_DEFS_H

#include <foe/simulation/type_defs.h>

#define FOE_GRAPHICS_RESOURCE_FUNCTIONALITY_ID FOE_SIMULATION_FUNCTIONALITY_ID(1)

enum foeGraphicsResourceStructureType {
    // Image
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE = FOE_GRAPHICS_RESOURCE_FUNCTIONALITY_ID,
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER,
    // Material
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL,
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_CREATE_INFO,
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER,
    // Mesh
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH,
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_FILE_CREATE_INFO,
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_CUBE_CREATE_INFO,
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_ICOSPHERE_CREATE_INFO,
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER,
    // Shader
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER,
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_CREATE_INFO,
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER,
    // VertexDescriptor
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR,
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_CREATE_INFO,
    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER,
};

#endif // FOE_GRAPHICS_RESOURCE_TYPE_DEFS_H