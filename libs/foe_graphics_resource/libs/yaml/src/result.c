// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/yaml/result.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foeGraphicsResourceYamlResultToString(foeGraphicsResourceYamlResult value,
                                           char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_YAML_SUCCESS)
        // Exporter
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_RESOURCE_EXPORTER)
        // Image Resource
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_YAML_ERROR_IMAGE_POOL_NOT_FOUND)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_YAML_ERROR_IMAGE_RESOURCE_ALREADY_EXISTS)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_IMAGE_IMPORTER)
        // Material Resource
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_YAML_ERROR_MATERIAL_POOL_NOT_FOUND)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_YAML_ERROR_MATERIAL_RESOURCE_ALREADY_EXISTS)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MATERIAL_IMPORTER)
        // Mesh Resource
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_YAML_ERROR_MESH_POOL_NOT_FOUND)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_YAML_ERROR_MESH_RESOURCE_ALREADY_EXISTS)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MESH_IMPORTER)
        // Vertex Descriptor Resource
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_YAML_ERROR_VERTEX_DESCRIPTOR_POOL_NOT_FOUND)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_YAML_ERROR_VERTEX_DESCRIPTOR_RESOURCE_ALREADY_EXISTS)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_VERTEX_DESCRIPTOR_IMPORTER)

    default:
        if (value > 0) {
            sprintf(buffer, "FOE_GRAPHICS_RESOURCE_YAML_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "FOE_GRAPHICS_RESOURCE_YAML_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}