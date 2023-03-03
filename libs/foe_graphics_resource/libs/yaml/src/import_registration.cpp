// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/yaml/import_registration.h>

#include <foe/imex/importer.h>
#include <foe/imex/yaml/importer.hpp>

#include "image.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "result.h"
#include "shader.hpp"
#include "vertex_descriptor.hpp"

extern "C" foeResultSet foeGraphicsResourceYamlRegisterImporters() {
    foeResultSet result = to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_SUCCESS);

    // Resources
    if (!foeImexYamlRegisterResourceFns(yaml_image_key(), yaml_read_image)) {
        result = to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_IMAGE_IMPORTER);
        goto REGISTRATION_FAILED;
    }
    if (!foeImexYamlRegisterResourceFns(yaml_material_key(), yaml_read_material)) {
        result =
            to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MATERIAL_IMPORTER);
        goto REGISTRATION_FAILED;
    }

    if (!foeImexYamlRegisterResourceFns(yaml_mesh_file_key(), yaml_read_mesh_file)) {
        result = to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MESH_IMPORTER);
        goto REGISTRATION_FAILED;
    }

    if (!foeImexYamlRegisterResourceFns(yaml_mesh_cube_key(), yaml_read_mesh_cube)) {
        result = to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MESH_IMPORTER);
        goto REGISTRATION_FAILED;
    }

    if (!foeImexYamlRegisterResourceFns(yaml_mesh_icosphere_key(), yaml_read_mesh_icosphere)) {
        result = to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MESH_IMPORTER);
        goto REGISTRATION_FAILED;
    }

    if (!foeImexYamlRegisterResourceFns(yaml_shader_key(), yaml_read_shader)) {
        result = to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_SHADER_IMPORTER);
        goto REGISTRATION_FAILED;
    }
    if (!foeImexYamlRegisterResourceFns(yaml_vertex_descriptor_key(),
                                        yaml_read_vertex_descriptor)) {
        result = to_foeResult(
            FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_VERTEX_DESCRIPTOR_IMPORTER);
        goto REGISTRATION_FAILED;
    }

REGISTRATION_FAILED:
    if (result.value != FOE_SUCCESS)
        foeGraphicsResourceYamlDeregisterImporters();

    return result;
}

extern "C" void foeGraphicsResourceYamlDeregisterImporters() {
    // Resources
    foeImexYamlDeregisterResourceFns(yaml_vertex_descriptor_key(), yaml_read_vertex_descriptor);
    foeImexYamlDeregisterResourceFns(yaml_shader_key(), yaml_read_shader);
    foeImexYamlDeregisterResourceFns(yaml_mesh_file_key(), yaml_read_mesh_file);
    foeImexYamlDeregisterResourceFns(yaml_mesh_cube_key(), yaml_read_mesh_cube);
    foeImexYamlDeregisterResourceFns(yaml_mesh_icosphere_key(), yaml_read_mesh_icosphere);
    foeImexYamlDeregisterResourceFns(yaml_material_key(), yaml_read_material);
    foeImexYamlDeregisterResourceFns(yaml_image_key(), yaml_read_image);
}