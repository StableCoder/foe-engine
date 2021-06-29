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

#include <foe/graphics/resource/yaml/export_registrar.hpp>

#include <foe/graphics/resource/material.hpp>
#include <foe/graphics/resource/material_loader.hpp>
#include <foe/graphics/resource/material_pool.hpp>
#include <foe/imex/exporters.hpp>
#include <foe/imex/yaml/exporter.hpp>

#include "error_code.hpp"
#include "material.hpp"

namespace {

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

void onDeregister(foeExporterBase *pExporter) {
    auto *pYamlExporter = dynamic_cast<foeYamlExporter *>(pExporter);
    if (pYamlExporter) {
        // Resource
        pYamlExporter->deregisterResourceFn(exportMaterial);
    }
}

std::error_code onRegister(foeExporterBase *pExporter) {
    std::error_code errC;

    auto *pYamlExporter = dynamic_cast<foeYamlExporter *>(pExporter);
    if (pYamlExporter) {
        // Resource
        if (!pYamlExporter->registerResourceFn(exportMaterial)) {
            errC = FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MATERIAL_EXPORTER;
            goto REGISTRATION_FAILED;
        }
    }

REGISTRATION_FAILED:
    if (errC)
        onDeregister(pExporter);

    return errC;
}

} // namespace

auto foeGraphicsResourceYamlRegisterExportFunctionality() -> std::error_code {
    return foeRegisterExportFunctionality(foeExportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}

void foeGraphicsResourceYamlDeregisterExportFunctionality() {
    foeDeregisterExportFunctionality(foeExportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}