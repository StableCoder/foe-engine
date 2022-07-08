// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_MESH_CREATE_INFO_HPP
#define FOE_GRAPHICS_RESOURCE_MESH_CREATE_INFO_HPP

#include <foe/graphics/resource/export.h>
#include <foe/resource/create_info.h>

#include <memory>
#include <string>

struct foeMeshSource {
    virtual ~foeMeshSource() = default;
};

struct foeMeshFileSource : public foeMeshSource {
    std::string fileName;
    std::string meshName;
    unsigned int postProcessFlags;
};

struct foeMeshCubeSource : public foeMeshSource {};

struct foeMeshIcosphereSource : public foeMeshSource {
    int recursion;
};

struct foeMeshCreateInfo {
    std::unique_ptr<foeMeshSource> source;
};

FOE_GFX_RES_EXPORT void foeDestroyMeshCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo);

#endif // FOE_GRAPHICS_RESOURCE_MESH_CREATE_INFO_HPP