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

#ifndef FOE_GRAPHICS_RESOURCE_MESH_CREATE_INFO_HPP
#define FOE_GRAPHICS_RESOURCE_MESH_CREATE_INFO_HPP

#include <foe/graphics/resource/export.h>
#include <foe/resource/create_info.h>

#include <memory>

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