/*
    Copyright (C) 2021-2022 George Cave.

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

#include <foe/graphics/resource/registration.h>
#include <foe/physics/registration.h>
#include <foe/position/registration.h>
#include <foe/simulation/simulation.hpp>

#include <foe/imex/yaml/exporter_registration.h>
#include <foe/imex/yaml/importer_registration.h>

#include <foe/graphics/resource/yaml/export_registration.h>
#include <foe/graphics/resource/yaml/import_registration.h>
#include <foe/physics/yaml/export_registration.h>
#include <foe/physics/yaml/import_registration.h>
#include <foe/position/yaml/export_registration.h>
#include <foe/position/yaml/import_registration.h>

#include "simulation/registration.hpp"
#include "simulation/yaml/export_registration.hpp"
#include "simulation/yaml/import_registration.hpp"

auto registerBasicFunctionality() noexcept -> std::error_code {
    std::error_code errC;

    // Core
    errC = foePhysicsRegisterFunctionality();
    if (errC)
        return errC;

    errC = foePositionRegisterFunctionality();
    if (errC)
        return errC;

    errC = foeGraphicsResourceRegisterFunctionality();
    if (errC)
        return errC;

    errC = foeBringupRegisterFunctionality();
    if (errC)
        return errC;

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

    errC = foeBringupYamlRegisterExporters();
    if (errC)
        return errC;

    // Import
    errC = foeRegisterYamlImportGenerator();
    if (errC)
        return errC;

    errC = foePhysicsYamlRegisterImporters();
    if (errC)
        return errC;

    errC = foePositionYamlRegisterImporters();
    if (errC)
        return errC;

    errC = foeGraphicsResourceYamlRegisterImporters();
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
    foeGraphicsResourceYamlDeregisterImporters();
    foePositionYamlDeregisterImporters();
    foePhysicsYamlDeregisterImporters();

    foeDeregisterYamlImportGenerator();

    // Export
    foeBringupYamlDeregisterExporters();
    foeGraphicsResourceYamlDeregisterExporters();
    foePositionYamlDeregisterExporters();
    foePhysicsYamlDeregisterExporters();

    foeImexYamlDeregisterExporter();

    // Core
    foeBringupDeregisterFunctionality();
    foeGraphicsResourceDeregisterFunctionality();
    foePhysicsDeregisterFunctionality();
    foePositionDeregisterFunctionality();
}