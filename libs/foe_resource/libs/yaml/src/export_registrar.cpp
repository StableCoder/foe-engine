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

#include <foe/resource/yaml/export_registrar.hpp>

#include <foe/imex/exporters.hpp>
#include <foe/imex/yaml/exporter.hpp>

#include <foe/resource/armature_pool.hpp>
#include <foe/resource/mesh_pool.hpp>
#include <foe/resource/shader_pool.hpp>

#include "armature.hpp"
#include "error_code.hpp"
#include "mesh.hpp"
#include "shader.hpp"

namespace {

std::vector<foeKeyYamlPair> exportResources(foeResourceID resource,
                                            foeResourcePoolBase **pResourcePools,
                                            uint32_t resourcePoolCount) {
    std::vector<foeKeyYamlPair> keyDataPairs;
    auto const *pEndPools = pResourcePools + resourcePoolCount;

    for (; pResourcePools != pEndPools; ++pResourcePools) {

        // Armature
        auto *pArmaturePool = dynamic_cast<foeArmaturePool *>(*pResourcePools);
        if (pArmaturePool) {
            auto const *pArmature = pArmaturePool->find(resource);
            if (pArmature && pArmature->createInfo) {
                keyDataPairs.emplace_back(foeKeyYamlPair{
                    .key = yaml_armature_key(),
                    .data = yaml_write_armature(*pArmature->createInfo.get()),
                });
            }
        }

        // Mesh
        auto *pMeshPool = dynamic_cast<foeMeshPool *>(*pResourcePools);
        if (pMeshPool) {
            auto const *pMesh = pMeshPool->find(resource);
            if (pMesh && pMesh->createInfo) {
                keyDataPairs.emplace_back(foeKeyYamlPair{
                    .key = yaml_mesh_key(),
                    .data = yaml_write_mesh(*pMesh->createInfo.get()),
                });
            }
        }

        // Shader
        auto *pShaderPool = dynamic_cast<foeShaderPool *>(*pResourcePools);
        if (pShaderPool) {
            auto const *pShader = pShaderPool->find(resource);
            if (pShader && pShader->createInfo) {
                keyDataPairs.emplace_back(foeKeyYamlPair{
                    .key = yaml_shader_key(),
                    .data = yaml_write_shader(*pShader->createInfo.get()),
                });
            }
        }
    }

    return keyDataPairs;
}

void onDeregister(foeExporterBase *pExporter) {
    auto *pYamlExporter = dynamic_cast<foeYamlExporter *>(pExporter);
    if (pYamlExporter) {
        // Resource
        pYamlExporter->deregisterResourceFn(exportResources);
    }
}

std::error_code onRegister(foeExporterBase *pExporter) {
    std::error_code errC;

    auto *pYamlExporter = dynamic_cast<foeYamlExporter *>(pExporter);
    if (pYamlExporter) {
        // Resource
        if (!pYamlExporter->registerResourceFn(exportResources)) {
            errC = FOE_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_RESOURCE_EXPORTERS;
            goto REGISTRATION_FAILED;
        }
    }

REGISTRATION_FAILED:
    if (errC)
        onDeregister(pExporter);

    return errC;
}

} // namespace

auto foeResourceRegisterYamlExportFunctionality() -> std::error_code {
    return foeRegisterExportFunctionality(foeExportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}

void foeResourceDeregisterYamlExportFunctionality() {
    foeDeregisterExportFunctionality(foeExportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}