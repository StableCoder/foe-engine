// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "register_basic_functionality.hpp"

#include <foe/plugin.h>

#include <foe/graphics/resource/registration.h>
#include <foe/physics/registration.h>
#include <foe/position/registration.h>
#include <foe/simulation/simulation.hpp>

#include "result.h"
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
        .path = BRINGUP_YAML,
        .initFn = {"foeBringupYamlRegisterExporters", "foeBringupYamlRegisterImporters"},
        .deinitFn = {"foeBringupYamlDeregisterImporters", "foeBringupYamlDeregisterExporters"},
    },
};

foeResultSet initItem(ImExPlugin &plugin) {
    foeCreatePlugin(plugin.path.c_str(), &plugin.plugin);
    if (plugin.plugin == FOE_NULL_HANDLE) {
        return to_foeResult(FOE_BRINGUP_FAILED_TO_LOAD_PLUGIN);
    }

    foeResultSet result = {.value = FOE_SUCCESS, .toString = NULL};
    for (auto const &it : plugin.initFn) {
        foeResultSet (*pFn)() = (foeResultSet(*)())foeGetPluginSymbol(plugin.plugin, it.c_str());
        foeResultSet fnResult = pFn();
        if (fnResult.value != FOE_SUCCESS) {
            result = fnResult;
            break;
        }
    }

    return result;
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

foeResultSet registerBasicFunctionality() noexcept {
    foeResultSet result;

    // Core
    result = foePhysicsRegisterFunctionality();
    if (result.value != FOE_SUCCESS)
        return result;

    result = foePositionRegisterFunctionality();
    if (result.value != FOE_SUCCESS)
        return result;

    result = foeGraphicsResourceRegisterFunctionality();
    if (result.value != FOE_SUCCESS)
        return result;

    result = foeBringupRegisterFunctionality();
    if (result.value != FOE_SUCCESS)
        return result;

    // Plugins
    for (auto &it : pluginList) {
        result = initItem(it);
        if (result.value != FOE_SUCCESS)
            break;
    }

    return result;
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