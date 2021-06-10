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

#include <foe/position/yaml/export_registrar.hpp>

#include <foe/imex/exporters.hpp>
#include <foe/imex/yaml/exporter.hpp>
#include <foe/position/component/3d_pool.hpp>
#include <foe/position/yaml/component/3d.hpp>

namespace {

std::vector<foeKeyYamlPair> exportComponents(foeEntityID entity,
                                             foeComponentPoolBase **pComponentPools,
                                             uint32_t componentPoolCount) {
    std::vector<foeKeyYamlPair> keyDataPairs;
    auto const *pEndPools = pComponentPools + componentPoolCount;

    for (; pComponentPools != pEndPools; ++pComponentPools) {
        auto *pPosition3dPool = dynamic_cast<foePosition3dPool *>(*pComponentPools);
        if (pPosition3dPool) {
            if (auto searchIt = pPosition3dPool->find(entity);
                searchIt != pPosition3dPool->size()) {
                keyDataPairs.emplace_back(foeKeyYamlPair{
                    .key = "position_3d",
                    .data = yaml_write_Position3D(*pPosition3dPool->begin<1>()[searchIt].get()),
                });
            }
        }
    }

    return keyDataPairs;
}

void onRegister(foeExporterBase *pExporter) {
    auto *pYamlExporter = dynamic_cast<foeYamlExporter *>(pExporter);
    if (pYamlExporter) {
        // Component
        pYamlExporter->registerComponentFn(exportComponents);
    }
}

void onDeregister(foeExporterBase *pExporter) {
    auto *pYamlExporter = dynamic_cast<foeYamlExporter *>(pExporter);
    if (pYamlExporter) {
        // Component
        pYamlExporter->deregisterComponentFn(exportComponents);
    }
}

} // namespace

void foePositionRegisterYamlExportFunctionality() {
    foeRegisterExportFunctionality(foeExportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}

void foePositionDeregisterYamlExportFunctionality() {
    foeDeregisterExportFunctionality(foeExportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}