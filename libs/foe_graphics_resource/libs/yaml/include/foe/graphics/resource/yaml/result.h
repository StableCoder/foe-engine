// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_YAML_RESULT_H
#define FOE_GRAPHICS_RESOURCE_YAML_RESULT_H

#include <foe/graphics/resource/yaml/export.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeGraphicsResourceYamlResult {
    FOE_GRAPHICS_RESOURCE_YAML_SUCCESS = 0,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_RESOURCE_EXPORTER = -1,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_IMAGE_POOL_NOT_FOUND = -2,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_IMAGE_RESOURCE_ALREADY_EXISTS = -3,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_IMAGE_IMPORTER = -4,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_MATERIAL_POOL_NOT_FOUND = -5,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_MATERIAL_RESOURCE_ALREADY_EXISTS = -6,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MATERIAL_IMPORTER = -7,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_MESH_POOL_NOT_FOUND = -8,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_MESH_RESOURCE_ALREADY_EXISTS = -9,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MESH_IMPORTER = -10,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_VERTEX_DESCRIPTOR_POOL_NOT_FOUND = -11,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_VERTEX_DESCRIPTOR_RESOURCE_ALREADY_EXISTS = -12,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_VERTEX_DESCRIPTOR_IMPORTER = -13,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_SHADER_POOL_NOT_FOUND = -14,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_SHADER_RESOURCE_ALREADY_EXISTS = -15,
    FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_SHADER_IMPORTER = -16,
} foeGraphicsResourceYamlResult;

FOE_GFX_RES_YAML_EXPORT void foeGraphicsResourceYamlResultToString(
    foeGraphicsResourceYamlResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_RESOURCE_YAML_RESULT_H