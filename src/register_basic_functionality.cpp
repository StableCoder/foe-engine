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

#include "register_basic_functionality.hpp"

#include <foe/graphics/resource/registrar.hpp>
#include <foe/physics/registrar.hpp>
#include <foe/position/registrar.hpp>
#include <foe/resource/registrar.hpp>
#include <foe/simulation/simulation.hpp>

#include <foe/graphics/resource/yaml/export_registration.hpp>
#include <foe/imex/yaml/exporter2.hpp>
#include <foe/physics/yaml/export_registration.hpp>
#include <foe/position/yaml/export_registration.hpp>
#include <foe/resource/yaml/export_registration.hpp>

#include <foe/graphics/resource/yaml/import_registration.hpp>
#include <foe/imex/yaml/generator.hpp>
#include <foe/physics/yaml/import_registration.hpp>
#include <foe/position/yaml/import_registration.hpp>
#include <foe/resource/yaml/import_registration.hpp>

#include "registrar.hpp"
#include "yaml_export_registration.hpp"
#include "yaml_import_registration.hpp"

auto registerBasicFunctionality() noexcept -> std::error_code {
    std::error_code errC;

    // Core
    foePhysicsRegisterFunctionality();
    foePositionRegisterFunctionality();
    foeResourceRegisterFunctionality();
    foeGraphicsResourceRegisterFunctionality();
    foeBringupRegisterFunctionality();

    // Export
    errC = foeImexYamlRegisterExporter();
    if (errC)
        return errC;

    errC = foePhysicsYamlRegisterExporters();
    if (errC)
        return errC;

    errC = foePositionYamlRegisterExporters();
    if (errC)
        return errC;

    errC = foeGraphicsResourceYamlRegisterExporters();
    if (errC)
        return errC;

    errC = foeResourceYamlRegisterExporters();
    if (errC)
        return errC;

    errC = foeBringupYamlRegisterExporters();
    if (errC)
        return errC;

    // Import
    foeRegisterYamlImportGenerator();

    errC = foePhysicsYamlRegisterImporters();
    if (errC)
        return errC;

    errC = foePositionYamlRegisterImporters();
    if (errC)
        return errC;

    errC = foeGraphicsResourceYamlRegisterImporters();
    if (errC)
        return errC;

    errC = foeResourceYamlRegisterImporters();
    if (errC)
        return errC;

    errC = foeBringupYamlRegisterImporters();
    if (errC)
        return errC;

    return errC;
}

void deregisterBasicFunctionality() noexcept {
    // Import
    foeBringupYamlDeregisterImporters();
    foeResourceYamlDeregisterImporters();
    foeGraphicsResourceYamlDeregisterImporters();
    foePositionYamlDeregisterImporters();
    foePhysicsYamlDeregisterImporters();

    foeDeregisterYamlImportGenerator();

    // Export
    foeBringupYamlDeregisterExporters();
    foeResourceYamlDeregisterExporters();
    foeGraphicsResourceYamlDeregisterExporters();
    foePositionYamlDeregisterExporters();
    foePhysicsYamlDeregisterExporters();

    foeImexYamlDeregisterExporter();

    // Core
    foeBringupDeregisterFunctionality();
    foeResourceDeregisterFunctionality();
    foeGraphicsResourceDeregisterFunctionality();
    foePhysicsDeregisterFunctionality();
    foePositionDeregisterFunctionality();
}