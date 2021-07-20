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

#include <foe/resource/yaml/import_registrar.hpp>

#include <foe/imex/yaml/generator.hpp>
#include <foe/resource/armature_loader.hpp>
#include <foe/resource/armature_pool.hpp>

#include "armature.hpp"
#include "error_code.hpp"

namespace {

std::error_code armatureCreateProcessing(foeResourceID resource,
                                         foeResourceCreateInfoBase *pCreateInfo,
                                         std::vector<foeResourcePoolBase *> &resourcePools) {
    foeArmaturePool *pArmaturePool{nullptr};
    for (auto &it : resourcePools) {
        pArmaturePool = dynamic_cast<foeArmaturePool *>(it);

        if (pArmaturePool != nullptr)
            break;
    }

    if (pArmaturePool == nullptr)
        return FOE_RESOURCE_YAML_ERROR_ARMATURE_POOL_NOT_FOUND;

    auto *pArmature = pArmaturePool->add(resource);

    if (!pArmature)
        return FOE_RESOURCE_YAML_ERROR_ARMATURE_RESOURCE_ALREADY_EXISTS;

    return FOE_RESOURCE_YAML_SUCCESS;
}

void onDeregister(foeImporterGenerator *pGenerator) {
    if (auto pYamlImporter = dynamic_cast<foeYamlImporterGenerator *>(pGenerator); pYamlImporter) {
        // Resources
        pYamlImporter->deregisterResourceFns(yaml_armature_key(), yaml_read_armature,
                                             armatureCreateProcessing);
    }
}

std::error_code onRegister(foeImporterGenerator *pGenerator) {
    std::error_code errC;

    if (auto pYamlImporter = dynamic_cast<foeYamlImporterGenerator *>(pGenerator); pYamlImporter) {
        // Resources
        if (!pYamlImporter->registerResourceFns(yaml_armature_key(), yaml_read_armature,
                                                armatureCreateProcessing)) {
            errC = FOE_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_ARMATURE_IMPORTER;
            goto REGISTRATION_FAILED;
        }
    }

REGISTRATION_FAILED:
    if (errC)
        onDeregister(pGenerator);

    return errC;
}

} // namespace

auto foeResourceRegisterYamlImportFunctionality() -> std::error_code {
    return foeRegisterImportFunctionality(foeImportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}

void foeResourceDeregisterYamlImportFunctionality() {
    foeDeregisterImportFunctionality(foeImportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}