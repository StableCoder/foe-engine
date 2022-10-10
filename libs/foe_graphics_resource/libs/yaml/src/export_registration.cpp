// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/yaml/export_registration.h>

#include <foe/graphics/resource/image.hpp>
#include <foe/graphics/resource/material.hpp>
#include <foe/graphics/resource/mesh.hpp>
#include <foe/graphics/resource/shader.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/resource/vertex_descriptor.hpp>
#include <foe/imex/exporters.h>
#include <foe/imex/yaml/exporter.hpp>
#include <foe/resource/pool.h>

#include "image.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "result.h"
#include "shader.hpp"
#include "vertex_descriptor.hpp"

namespace {

std::vector<foeKeyYamlPair> exportResource(foeResourceCreateInfo createInfo) {
    std::vector<foeKeyYamlPair> keyDataPairs;

    if (createInfo == FOE_NULL_HANDLE)
        return keyDataPairs;

    if (foeResourceCreateInfoGetType(createInfo) ==
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_CREATE_INFO) {
        auto const *pCreateInfo =
            (foeImageCreateInfo const *)foeResourceCreateInfoGetData(createInfo);

        keyDataPairs.emplace_back(foeKeyYamlPair{
            .key = yaml_image_key(),
            .data = yaml_write_image(*pCreateInfo),
        });
    }

    if (foeResourceCreateInfoGetType(createInfo) ==
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_CREATE_INFO) {
        auto const *pCreateInfo =
            (foeMaterialCreateInfo const *)foeResourceCreateInfoGetData(createInfo);

        keyDataPairs.emplace_back(foeKeyYamlPair{
            .key = yaml_material_key(),
            .data = yaml_write_material(*pCreateInfo),
        });
    }

    if (foeResourceCreateInfoGetType(createInfo) ==
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_FILE_CREATE_INFO) {
        foeMeshFileCreateInfo const *pCreateInfo =
            (foeMeshFileCreateInfo const *)foeResourceCreateInfoGetData(createInfo);

        keyDataPairs.emplace_back(foeKeyYamlPair{
            .key = yaml_mesh_file_key(),
            .data = yaml_write_mesh_file(*pCreateInfo),
        });
    }

    if (foeResourceCreateInfoGetType(createInfo) ==
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_CUBE_CREATE_INFO) {
        foeMeshCubeCreateInfo const *pCreateInfo =
            (foeMeshCubeCreateInfo const *)foeResourceCreateInfoGetData(createInfo);

        keyDataPairs.emplace_back(foeKeyYamlPair{
            .key = yaml_mesh_cube_key(),
            .data = yaml_write_mesh_cube(*pCreateInfo),
        });
    }
    if (foeResourceCreateInfoGetType(createInfo) ==
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_ICOSPHERE_CREATE_INFO) {
        foeMeshIcosphereCreateInfo const *pCreateInfo =
            (foeMeshIcosphereCreateInfo const *)foeResourceCreateInfoGetData(createInfo);

        keyDataPairs.emplace_back(foeKeyYamlPair{
            .key = yaml_mesh_icosphere_key(),
            .data = yaml_write_mesh_icosphere(*pCreateInfo),
        });
    }

    if (foeResourceCreateInfoGetType(createInfo) ==
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_CREATE_INFO) {
        auto const *pCreateInfo =
            (foeShaderCreateInfo const *)foeResourceCreateInfoGetData(createInfo);

        keyDataPairs.emplace_back(foeKeyYamlPair{
            .key = yaml_shader_key(),
            .data = yaml_write_shader(*pCreateInfo),
        });
    }

    if (foeResourceCreateInfoGetType(createInfo) ==
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_CREATE_INFO) {
        auto const *pCreateInfo =
            (foeVertexDescriptorCreateInfo const *)foeResourceCreateInfoGetData(createInfo);

        keyDataPairs.emplace_back(foeKeyYamlPair{
            .key = yaml_vertex_descriptor_key(),
            .data = yaml_write_vertex_descriptor(*pCreateInfo),
        });
    }

    return keyDataPairs;
}

void onDeregister(foeExporter exporter) {
    if (std::string_view{exporter.pName} == "Yaml") {
        // Resource
        foeImexYamlDeregisterResourceFn(exportResource);
    }
}

foeResultSet onRegister(foeExporter exporter) {
    foeResultSet result = to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_SUCCESS);

    if (std::string_view{exporter.pName} == "Yaml") {
        // Resource
        result = foeImexYamlRegisterResourceFn(exportResource);
        if (result.value != FOE_SUCCESS) {
            result =
                to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_RESOURCE_EXPORTER);
            goto REGISTRATION_FAILED;
        }
    }

REGISTRATION_FAILED:
    if (result.value != FOE_SUCCESS)
        onDeregister(exporter);

    return result;
}

foeExportFunctionality exportFunctionality{
    .onRegister = onRegister,
    .onDeregister = onDeregister,
};

} // namespace

extern "C" foeResultSet foeGraphicsResourceYamlRegisterExporters() {
    return foeRegisterExportFunctionality(&exportFunctionality);
}

extern "C" void foeGraphicsResourceYamlDeregisterExporters() {
    foeDeregisterExportFunctionality(&exportFunctionality);
}