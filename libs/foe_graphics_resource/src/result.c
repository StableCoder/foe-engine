// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/result.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foeGraphicsResourceResultToString(foeGraphicsResourceResult value,
                                       char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_SUCCESS)
        // Loaders
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_INCOMPATIBLE_CREATE_INFO)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_INCOMPATIBLE_RESOURCE_TYPE)
        // Image Loader
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_IMAGE_LOADER_INITIALIZATION_FAILED)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_IMAGE_LOADER_NOT_INITIALIZED)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_EXTERNAL_IMAGE_FORMAT_UNKNOWN)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_EXTERNAL_IMAGE_LOAD_FAILURE)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_IMAGE_UPLOAD_FAILURE)
        // Material Loader
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_LOADER_INITIALIZATION_FAILED)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_LOADER_NOT_INITIALIZED)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_SUBRESOURCE_FAILED_TO_LOAD)
        // Mesh Loader
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_MESH_LOADER_INITIALIZATION_FAILED)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_MESH_LOADER_NOT_INITIALIZED)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_MESH_UPLOAD_FAILED)
        // Vertex Descriptor Loader
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_VERTEX_DESCRIPTOR_LOADER_INITIALIZATION_FAILED)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_VERTEX_DESCRIPTOR_SUBRESOURCE_FAILED_TO_LOAD)
        // Shader Loader
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_SHADER_LOADER_INITIALIZATION_FAILED)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_SHADER_LOADER_NOT_INITIALIZED)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_SHADER_LOADER_BINARY_FILE_NOT_FOUND)

    default:
        if (value > 0) {
            sprintf(buffer, "FOE_GRAPHICS_RESOURCE_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "FOE_GRAPHICS_RESOURCE_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}