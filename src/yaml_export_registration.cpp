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

#include "yaml_export_registration.hpp"

#include <foe/imex/exporters.hpp>
#include <foe/imex/yaml/exporter.hpp>

#include "armature_state_imex.hpp"
#include "armature_state_pool.hpp"
#include "camera_imex.hpp"
#include "camera_pool.hpp"
#include "error_code.hpp"
#include "render_state_imex.hpp"
#include "render_state_pool.hpp"

namespace {

std::vector<foeKeyYamlPair> exportComponents(foeEntityID entity,
                                             foeComponentPoolBase **pComponentPools,
                                             uint32_t componentPoolCount) {
    std::vector<foeKeyYamlPair> keyDataPairs;
    auto const *pEndPools = pComponentPools + componentPoolCount;

    for (; pComponentPools != pEndPools; ++pComponentPools) {

        // ArmatureState
        auto *pArmatureStatePool = dynamic_cast<foeArmatureStatePool *>(*pComponentPools);
        if (pArmatureStatePool) {
            if (auto searchIt = pArmatureStatePool->find(entity);
                searchIt != pArmatureStatePool->size()) {
                keyDataPairs.emplace_back(foeKeyYamlPair{
                    .key = yaml_armature_state_key(),
                    .data = yaml_write_ArmatureState(pArmatureStatePool->begin<1>()[searchIt]),
                });
            }
        }

        // Camera
        auto *pCameraPool = dynamic_cast<foeCameraPool *>(*pComponentPools);
        if (pCameraPool) {
            if (auto searchIt = pCameraPool->find(entity); searchIt != pCameraPool->size()) {
                keyDataPairs.emplace_back(foeKeyYamlPair{
                    .key = yaml_camera_key(),
                    .data = yaml_write_Camera(*pCameraPool->begin<1>()[searchIt].get()),
                });
            }
        }

        // RenderState
        auto *pRenderStatePool = dynamic_cast<foeRenderStatePool *>(*pComponentPools);
        if (pRenderStatePool) {
            if (auto searchIt = pRenderStatePool->find(entity);
                searchIt != pRenderStatePool->size()) {
                keyDataPairs.emplace_back(foeKeyYamlPair{
                    .key = yaml_render_state_key(),
                    .data = yaml_write_RenderState(pRenderStatePool->begin<1>()[searchIt]),
                });
            }
        }
    }

    return keyDataPairs;
}

void onDeregister(foeExporterBase *pExporter) {
    auto *pYamlExporter = dynamic_cast<foeYamlExporter *>(pExporter);
    if (pYamlExporter) {
        // Component
        pYamlExporter->deregisterComponentFn(exportComponents);
    }
}

std::error_code onRegister(foeExporterBase *pExporter) {
    std::error_code errC;

    auto *pYamlExporter = dynamic_cast<foeYamlExporter *>(pExporter);
    if (pYamlExporter) {
        // Component
        if (!pYamlExporter->registerComponentFn(exportComponents)) {
            errC = FOE_BRINGUP_ERROR_FAILED_TO_REGISTER_EXPORTERS;
            goto REGISTRATION_FAILED;
        }
    }

REGISTRATION_FAILED:
    if (errC)
        onDeregister(pExporter);

    return errC;
}

} // namespace

auto foeBringupYamlRegisterExporters() -> std::error_code {
    return foeRegisterExportFunctionality(foeExportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}

void foeBringupYamlDeregisterExporters() {
    foeDeregisterExportFunctionality(foeExportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}