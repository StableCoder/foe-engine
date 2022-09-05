// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "register_basic_functionality.hpp"

#include <foe/plugin.h>

#include <foe/graphics/resource/registration.h>
#include <foe/graphics/resource/yaml/export_registration.h>
#include <foe/graphics/resource/yaml/import_registration.h>
#include <foe/imex/yaml/exporter_registration.h>
#include <foe/imex/yaml/importer_registration.h>
#include <foe/physics/registration.h>
#include <foe/physics/yaml/export_registration.h>
#include <foe/physics/yaml/import_registration.h>
#include <foe/position/registration.h>
#include <foe/position/yaml/export_registration.h>
#include <foe/position/yaml/import_registration.h>
#include <foe/simulation/simulation.hpp>

#include "result.h"
#include "simulation/registration.hpp"
#include "simulation/yaml/export_registration.hpp"
#include "simulation/yaml/import_registration.hpp"

#include <array>
#include <string>

namespace {

std::array<foeResultSet (*)(), 14> pluginInitCalls = {
    // Core
    foePhysicsRegisterFunctionality,
    foePositionRegisterFunctionality,
    foeGraphicsResourceRegisterFunctionality,
    foeBringupRegisterFunctionality,
    // Yaml
    foeImexYamlRegisterExporter,
    foeImexYamlRegisterImporter,
    foePhysicsYamlRegisterExporters,
    foePhysicsYamlRegisterImporters,
    foePositionYamlRegisterExporters,
    foePositionYamlRegisterImporters,
    foeGraphicsResourceYamlRegisterExporters,
    foeGraphicsResourceYamlRegisterImporters,
    foeBringupYamlRegisterExporters,
    foeBringupYamlRegisterImporters,
};

std::array<void (*)(), 14> pluginDeinitCalls = {
    // Yaml
    foeImexYamlDeregisterImporter,
    foeImexYamlDeregisterExporter,
    foePhysicsYamlDeregisterImporters,
    foePhysicsYamlDeregisterExporters,
    foePositionYamlDeregisterImporters,
    foePositionYamlDeregisterExporters,
    foeGraphicsResourceYamlDeregisterImporters,
    foeGraphicsResourceYamlDeregisterExporters,
    foeBringupYamlDeregisterImporters,
    foeBringupYamlDeregisterExporters,
    // Core
    foeBringupDeregisterFunctionality,
    foeGraphicsResourceDeregisterFunctionality,
    foePhysicsDeregisterFunctionality,
    foePositionDeregisterFunctionality,
};

} // namespace

foeResultSet registerBasicFunctionality() noexcept {
    foeResultSet result = to_foeResult(FOE_BRINGUP_SUCCESS);

    // Plugins
    for (auto &it : pluginInitCalls) {
        result = it();
        if (result.value != FOE_SUCCESS)
            break;
    }

    return result;
}

void deregisterBasicFunctionality() noexcept {
    // Plugins
    for (auto &it : pluginDeinitCalls) {
        it();
    }
}