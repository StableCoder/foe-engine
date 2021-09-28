/*
    Copyright (C) 2021 George Cave.

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

#include <foe/graphics/resource/yaml/export_registration.hpp>

#include <foe/graphics/resource/image.hpp>
#include <foe/graphics/resource/image_loader.hpp>
#include <foe/graphics/resource/image_pool.hpp>
#include <foe/graphics/resource/material.hpp>
#include <foe/graphics/resource/material_loader.hpp>
#include <foe/graphics/resource/material_pool.hpp>
#include <foe/graphics/resource/mesh.hpp>
#include <foe/graphics/resource/mesh_loader.hpp>
#include <foe/graphics/resource/mesh_pool.hpp>
#include <foe/graphics/resource/shader.hpp>
#include <foe/graphics/resource/shader_loader.hpp>
#include <foe/graphics/resource/shader_pool.hpp>
#include <foe/graphics/resource/vertex_descriptor.hpp>
#include <foe/graphics/resource/vertex_descriptor_loader.hpp>
#include <foe/graphics/resource/vertex_descriptor_pool.hpp>
#include <foe/imex/exporters.hpp>
#include <foe/imex/yaml/exporter.hpp>

#include "error_code.hpp"
#include "image.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "vertex_descriptor.hpp"

namespace {

std::vector<foeKeyYamlPair> exportImage(foeResourceID resource,
                                        foeResourcePoolBase **pResourcePools,
                                        uint32_t resourcePoolCount) {
    std::vector<foeKeyYamlPair> keyDataPairs;
    auto const *pEndPools = pResourcePools + resourcePoolCount;

    for (; pResourcePools != pEndPools; ++pResourcePools) {
        auto *pImagePool = dynamic_cast<foeImagePool *>(*pResourcePools);
        if (pImagePool) {
            auto const *pImage = pImagePool->find(resource);
            if (pImage && pImage->pCreateInfo) {
                if (auto pImageCI = dynamic_cast<foeImageCreateInfo *>(pImage->pCreateInfo.get());
                    pImageCI)
                    keyDataPairs.emplace_back(foeKeyYamlPair{
                        .key = yaml_image_key(),
                        .data = yaml_write_image(*pImageCI),
                    });
            }
        }
    }

    return keyDataPairs;
}

std::vector<foeKeyYamlPair> exportMaterial(foeResourceID resource,
                                           foeResourcePoolBase **pResourcePools,
                                           uint32_t resourcePoolCount) {
    std::vector<foeKeyYamlPair> keyDataPairs;
    auto const *pEndPools = pResourcePools + resourcePoolCount;

    for (; pResourcePools != pEndPools; ++pResourcePools) {
        auto *pMaterialPool = dynamic_cast<foeMaterialPool *>(*pResourcePools);
        if (pMaterialPool) {
            auto const *pMaterial = pMaterialPool->find(resource);
            if (pMaterial && pMaterial->pCreateInfo) {
                if (auto pMaterialCI =
                        dynamic_cast<foeMaterialCreateInfo *>(pMaterial->pCreateInfo.get());
                    pMaterialCI)
                    keyDataPairs.emplace_back(foeKeyYamlPair{
                        .key = yaml_material_key(),
                        .data =
                            yaml_write_material(*pMaterialCI, pMaterial->data.pGfxFragDescriptor),
                    });
            }
        }
    }

    return keyDataPairs;
}

std::vector<foeKeyYamlPair> exportMesh(foeResourceID resource,
                                       foeResourcePoolBase **pResourcePools,
                                       uint32_t resourcePoolCount) {
    std::vector<foeKeyYamlPair> keyDataPairs;
    auto const *pEndPools = pResourcePools + resourcePoolCount;

    for (; pResourcePools != pEndPools; ++pResourcePools) {
        auto *pMesh2Pool = dynamic_cast<foeMeshPool *>(*pResourcePools);
        if (pMesh2Pool) {
            auto const *pMesh2 = pMesh2Pool->find(resource);
            if (pMesh2 && pMesh2->pCreateInfo) {
                if (auto pMesh2CI = dynamic_cast<foeMeshCreateInfo *>(pMesh2->pCreateInfo.get());
                    pMesh2CI)
                    keyDataPairs.emplace_back(foeKeyYamlPair{
                        .key = yaml_mesh_key(),
                        .data = yaml_write_mesh(*pMesh2CI),
                    });
            }
        }
    }

    return keyDataPairs;
}

std::vector<foeKeyYamlPair> exportShader(foeResourceID resource,
                                         foeResourcePoolBase **pResourcePools,
                                         uint32_t resourcePoolCount) {
    std::vector<foeKeyYamlPair> keyDataPairs;
    auto const *pEndPools = pResourcePools + resourcePoolCount;

    for (; pResourcePools != pEndPools; ++pResourcePools) {
        auto *pShaderPool = dynamic_cast<foeShaderPool *>(*pResourcePools);
        if (pShaderPool) {
            auto const *pShader = pShaderPool->find(resource);
            if (pShader && pShader->pCreateInfo) {
                if (auto pShaderCI =
                        dynamic_cast<foeShaderCreateInfo *>(pShader->pCreateInfo.get());
                    pShaderCI)
                    keyDataPairs.emplace_back(foeKeyYamlPair{
                        .key = yaml_shader_key(),
                        .data = yaml_write_shader(*pShaderCI),
                    });
            }
        }
    }

    return keyDataPairs;
}

std::vector<foeKeyYamlPair> exportVertexDescriptor(foeResourceID resource,
                                                   foeResourcePoolBase **pResourcePools,
                                                   uint32_t resourcePoolCount) {
    std::vector<foeKeyYamlPair> keyDataPairs;
    auto const *pEndPools = pResourcePools + resourcePoolCount;

    for (; pResourcePools != pEndPools; ++pResourcePools) {
        auto *pVertexDescriptor2Pool = dynamic_cast<foeVertexDescriptorPool *>(*pResourcePools);
        if (pVertexDescriptor2Pool) {
            auto const *pVertexDescriptor2 = pVertexDescriptor2Pool->find(resource);
            if (pVertexDescriptor2 && pVertexDescriptor2->pCreateInfo) {
                if (auto pVertexDescriptor2CI = dynamic_cast<foeVertexDescriptorCreateInfo *>(
                        pVertexDescriptor2->pCreateInfo.get());
                    pVertexDescriptor2CI)
                    keyDataPairs.emplace_back(foeKeyYamlPair{
                        .key = yaml_vertex_descriptor_key(),
                        .data = yaml_write_vertex_descriptor(*pVertexDescriptor2CI),
                    });
            }
        }
    }

    return keyDataPairs;
}

void onDeregister(foeExporter exporter) {
    if (std::string_view{exporter.pName} == "Yaml") {
        // Resource
        foeImexYamlDeregisterResourceFn(exportVertexDescriptor);
        foeImexYamlDeregisterResourceFn(exportShader);
        foeImexYamlDeregisterResourceFn(exportMesh);
        foeImexYamlDeregisterResourceFn(exportMaterial);
        foeImexYamlDeregisterResourceFn(exportImage);
    }
}

std::error_code onRegister(foeExporter exporter) {
    std::error_code errC;

    if (std::string_view{exporter.pName} == "Yaml") {
        // Resource
        if (foeImexYamlRegisterResourceFn(exportImage)) {
            errC = FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_IMAGE_EXPORTER;
            goto REGISTRATION_FAILED;
        }
        if (foeImexYamlRegisterResourceFn(exportMaterial)) {
            errC = FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MATERIAL_EXPORTER;
            goto REGISTRATION_FAILED;
        }
        if (foeImexYamlRegisterResourceFn(exportMesh)) {
            errC = FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MESH_EXPORTER;
            goto REGISTRATION_FAILED;
        }
        if (foeImexYamlRegisterResourceFn(exportShader)) {
            errC = FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_SHADER_EXPORTER;
            goto REGISTRATION_FAILED;
        }
        if (foeImexYamlRegisterResourceFn(exportVertexDescriptor)) {
            errC = FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_VERTEX_DESCRIPTOR_EXPORTER;
            goto REGISTRATION_FAILED;
        }
    }

REGISTRATION_FAILED:
    if (errC)
        onDeregister(exporter);

    return errC;
}

} // namespace

auto foeGraphicsResourceYamlRegisterExporters() -> std::error_code {
    return foeRegisterExportFunctionality(foeExportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}

void foeGraphicsResourceYamlDeregisterExporters() {
    foeDeregisterExportFunctionality(foeExportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}