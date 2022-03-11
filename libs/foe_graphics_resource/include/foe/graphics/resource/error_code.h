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

#ifndef FOE_GRAPHICS_RESOURCE_ERROR_CODE_H
#define FOE_GRAPHICS_RESOURCE_ERROR_CODE_H

#ifdef __cplusplus
extern "C" {
#endif

enum foeGraphicsResourceResult {
    FOE_GRAPHICS_RESOURCE_SUCCESS = 0,
    // Loaders
    FOE_GRAPHICS_RESOURCE_ERROR_FAILED_TO_FIND_COMPATIBLE_LOADER,
    FOE_GRAPHICS_RESOURCE_ERROR_INCOMPATIBLE_CREATE_INFO,
    // Image Loader
    FOE_GRAPHICS_RESOURCE_ERROR_IMAGE_LOADER_INITIALIZATION_FAILED,
    FOE_GRAPHICS_RESOURCE_ERROR_IMAGE_LOADER_NOT_INITIALIZED,
    FOE_GRAPHICS_RESOURCE_ERROR_EXTERNAL_IMAGE_FORMAT_UNKNOWN,
    FOE_GRAPHICS_RESOURCE_ERROR_EXTERNAL_IMAGE_LOAD_FAILURE,
    FOE_GRAPHICS_RESOURCE_ERROR_IMAGE_UPLOAD_FAILURE,
    // Material Loader
    FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_LOADER_INITIALIZATION_FAILED,
    FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_LOADER_NOT_INITIALIZED,
    FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_SUBRESOURCE_FAILED_TO_LOAD,
    // Mesh Loader
    FOE_GRAPHICS_RESOURCE_ERROR_MESH_LOADER_INITIALIZATION_FAILED,
    FOE_GRAPHICS_RESOURCE_ERROR_MESH_LOADER_NOT_INITIALIZED,
    FOE_GRAPHICS_RESOURCE_ERROR_MESH_UPLOAD_FAILED,
    // Vertex Descriptor Loader
    FOE_GRAPHICS_RESOURCE_ERROR_VERTEX_DESCRIPTOR_LOADER_INITIALIZATION_FAILED,
    FOE_GRAPHICS_RESOURCE_ERROR_VERTEX_DESCRIPTOR_SUBRESOURCE_FAILED_TO_LOAD,
    // Shader Loader
    FOE_GRAPHICS_RESOURCE_ERROR_SHADER_LOADER_INITIALIZATION_FAILED,
    FOE_GRAPHICS_RESOURCE_ERROR_SHADER_LOADER_NOT_INITIALIZED,
    FOE_GRAPHICS_RESOURCE_ERROR_SHADER_LOADER_BINARY_FILE_NOT_FOUND,
};

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_RESOURCE_ERROR_CODE_H