// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/mesh_create_info.hpp>

#include <foe/graphics/resource/type_defs.h>

void foeDestroyMeshCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo) {
    if (type == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_FILE_CREATE_INFO) {
        foeMeshFileCreateInfo *pCI = (foeMeshFileCreateInfo *)pCreateInfo;

        pCI->~foeMeshFileCreateInfo();
    } else if (type == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_CUBE_CREATE_INFO) {
        foeMeshCubeCreateInfo *pCI = (foeMeshCubeCreateInfo *)pCreateInfo;

        pCI->~foeMeshCubeCreateInfo();
    } else if (type == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_FILE_CREATE_INFO) {
        foeMeshIcosphereCreateInfo *pCI = (foeMeshIcosphereCreateInfo *)pCreateInfo;

        pCI->~foeMeshIcosphereCreateInfo();
    }
}