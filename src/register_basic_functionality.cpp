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

#include <foe/plugin.h>

#include <foe/graphics/resource/registration.h>
#include <foe/physics/registration.h>
#include <foe/position/registration.h>
#include <foe/simulation/simulation.hpp>

#include "error_code.hpp"
#include "simulation/registration.hpp"

#include <array>
#include <string>

namespace {

struct ImExPlugin {
    std::string path;
    std::vector<std::string> initFn;
    std::vector<std::string> deinitFn;
    foePlugin plugin;
};

std::array<ImExPlugin, 5> pluginList{
    ImExPlugin{
        .path = IMEX_YAML_LIB,
        .initFn = {"foeImexYamlRegisterExporter", "foeImexYamlRegisterImporter"},
        .deinitFn = {"foeImexYamlDeregisterImporter", "foeImexYamlDeregisterExporter"},
    },
    ImExPlugin{
        .path = PHYSICS_LIB,
        .initFn = {"foePhysicsYamlRegisterExporters", "foePhysicsYamlRegisterImporters"},
        .deinitFn = {"foePhysicsYamlDeregisterImporters", "foePhysicsYamlDeregisterExporters"},
    },
    ImExPlugin{
        .path = POSITION_LIB,
        .initFn = {"foePositionYamlRegisterExporters", "foePositionYamlRegisterImporters"},
        .deinitFn = {"foePositionYamlDeregisterImporters", "foePositionYamlDeregisterExporters"},
    },
    ImExPlugin{
        .path = GRAPHICS_RESOURCE_LIB,
        .initFn = {"foeGraphicsResourceYamlRegisterExporters",
                   "foeGraphicsResourceYamlRegisterImporters"},
        .deinitFn = {"foeGraphicsResourceYamlDeregisterImporters",
                     "foeGraphicsResourceYamlDeregisterExporters"},
    },
    ImExPlugin{
        .path = ENGINE_YAML,
        .initFn = {"foeBringupYamlRegisterExporters", "foeBringupYamlRegisterImporters"},
        .deinitFn = {"foeBringupYamlDeregisterImporters", "foeBringupYamlDeregisterExporters"},
    },
};

std::error_code initItem(ImExPlugin &plugin) {
    foeCreatePlugin(plugin.path.c_str(), &plugin.plugin);
    if (plugin.plugin == FOE_NULL_HANDLE) {
        return FOE_BRINGUP_FAILED_TO_LOAD_PLUGIN;
    }

    std::error_code errC = FOE_BRINGUP_SUCCESS;
    for (auto const &it : plugin.initFn) {
        foeErrorCode (*pFn)() = (foeErrorCode(*)())foeGetPluginSymbol(plugin.plugin, it.c_str());
        std::error_code fnErrC = pFn();
        if (fnErrC) {
            errC = fnErrC;
            break;
        }
    }

    return errC;
}

void deinitItem(ImExPlugin &plugin) {
    if (plugin.plugin != FOE_NULL_HANDLE) {
        for (auto const &it : plugin.deinitFn) {
            void (*pFn)() = (void (*)())foeGetPluginSymbol(plugin.plugin, it.c_str());
            pFn();
        }

        foeDestroyPlugin(plugin.plugin);
        plugin.plugin = FOE_NULL_HANDLE;
    }
}

} // namespace

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

    // Plugins
    for (auto &it : pluginList) {
        errC = initItem(it);
        if (errC)
            break;
    }

    return errC;
}

void deregisterBasicFunctionality() noexcept {
    // Plugins
    for (auto &it : pluginList) {
        deinitItem(it);
    }

    // Core
    foeBringupDeregisterFunctionality();
    foeGraphicsResourceDeregisterFunctionality();
    foePhysicsDeregisterFunctionality();
    foePositionDeregisterFunctionality();
}