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

#ifndef FOE_GRAPHICS_RESOURCE_YAML_ERROR_CODE_H
#define FOE_GRAPHICS_RESOURCE_YAML_ERROR_CODE_H

#include <foe/error_code.h>
#include <foe/graphics/resource/yaml/export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeGraphicsResourceYamlResult {
    FOE_GRAPHICS_RESOURCE_YAML_SUCCESS = 0,
    // Exporter
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_RESOURCE_EXPORTER,
    // Image Resource
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_IMAGE_POOL_NOT_FOUND,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_IMAGE_RESOURCE_ALREADY_EXISTS,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_IMAGE_IMPORTER,
    // Material Resource
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_MATERIAL_POOL_NOT_FOUND,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_MATERIAL_RESOURCE_ALREADY_EXISTS,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MATERIAL_IMPORTER,
    // Mesh Resource
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_MESH_POOL_NOT_FOUND,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_MESH_RESOURCE_ALREADY_EXISTS,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MESH_IMPORTER,
    // Vertex Descriptor Resource
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_VERTEX_DESCRIPTOR_POOL_NOT_FOUND,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_VERTEX_DESCRIPTOR_RESOURCE_ALREADY_EXISTS,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_VERTEX_DESCRIPTOR_IMPORTER,
    // Shader Resource
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_SHADER_POOL_NOT_FOUND,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_SHADER_RESOURCE_ALREADY_EXISTS,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_SHADER_IMPORTER,

    // Need to have a negative enum value to prevent treatment as a flag
    FOE_GRAPHICS_RESOURCE_YAML_NEGATIVE_VALUE = FOE_RESULT_MIN_ENUM,
} foeGraphicsResourceYamlResult;

FOE_GFX_RES_YAML_EXPORT void foeGraphicsResourceYamlResultToString(
    foeGraphicsResourceYamlResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_RESOURCE_YAML_ERROR_CODE_H