// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "register_basic_functionality.h"

#include <foe/plugin.h>

#include <foe/graphics/resource/registration.h>
#include <foe/physics/registration.h>
#include <foe/position/registration.h>
#include <foe/simulation/simulation.hpp>

#include "result.h"
#include "simulation/registration.h"

#include <array>
#include <string>
#include <vector>

namespace {

struct ImExPlugin {
    std::string path;
    std::vector<std::string> initFn;
    std::vector<std::string> deinitFn;
    foePlugin plugin;
};

std::array<ImExPlugin, 10> pluginList{
    // Yaml
    ImExPlugin{
        .path = IMEX_YAML_PLUGIN,
        .initFn = {"foeImexYamlRegisterExporter", "foeImexYamlRegisterImporter"},
        .deinitFn = {"foeImexYamlDeregisterImporter", "foeImexYamlDeregisterExporter"},
    },
    ImExPlugin{
        .path = PHYSICS_YAML_PLUGIN,
        .initFn = {"foePhysicsYamlRegisterExporters", "foePhysicsYamlRegisterImporters"},
        .deinitFn = {"foePhysicsYamlDeregisterImporters", "foePhysicsYamlDeregisterExporters"},
    },
    ImExPlugin{
        .path = POSITION_YAML_PLUGIN,
        .initFn = {"foePositionYamlRegisterExporters", "foePositionYamlRegisterImporters"},
        .deinitFn = {"foePositionYamlDeregisterImporters", "foePositionYamlDeregisterExporters"},
    },
    ImExPlugin{
        .path = GRAPHICS_RESOURCE_YAML_PLUGIN,
        .initFn = {"foeGraphicsResourceYamlRegisterExporters",
                   "foeGraphicsResourceYamlRegisterImporters"},
        .deinitFn = {"foeGraphicsResourceYamlDeregisterImporters",
                     "foeGraphicsResourceYamlDeregisterExporters"},
    },
    ImExPlugin{
        .path = BRINGUP_YAML_PLUGIN,
        .initFn = {"foeBringupYamlRegisterExporters", "foeBringupYamlRegisterImporters"},
        .deinitFn = {"foeBringupYamlDeregisterImporters", "foeBringupYamlDeregisterExporters"},
    },
    // Binary
    ImExPlugin{
        .path = IMEX_BINARY_PLUGIN,
        .initFn = {"foeImexBinaryRegisterExporter", "foeImexBinaryRegisterImporter"},
        .deinitFn = {"foeImexBinaryDeregisterImporter", "foeImexBinaryDeregisterExporter"},
    },
    ImExPlugin{
        .path = GRAPHICS_RESOURCE_BINARY_PLUGIN,
        .initFn = {"foeGraphicsResourceBinaryRegisterExporters",
                   "foeGraphicsResourceBinaryRegisterImporters"},
        .deinitFn = {"foeGraphicsResourceBinaryDeregisterImporters",
                     "foeGraphicsResourceBinaryDeregisterExporters"},
    },
    ImExPlugin{
        .path = PHYSICS_BINARY_PLUGIN,
        .initFn = {"foePhysicsBinaryRegisterExporters", "foePhysicsBinaryRegisterImporters"},
        .deinitFn = {"foePhysicsBinaryDeregisterImporters", "foePhysicsBinaryDeregisterExporters"},
    },
    ImExPlugin{
        .path = POSITION_BINARY_PLUGIN,
        .initFn = {"foePositionBinaryRegisterExporters", "foePositionBinaryRegisterImporters"},
        .deinitFn = {"foePositionBinaryDeregisterImporters",
                     "foePositionBinaryDeregisterExporters"},
    },
    ImExPlugin{
        .path = BRINGUP_BINARY_PLUGIN,
        .initFn = {"foeBringupBinaryRegisterExporters", "foeBringupBinaryRegisterImporters"},
        .deinitFn = {"foeBringupBinaryDeregisterImporters", "foeBringupBinaryDeregisterExporters"},
    },
};

foeResultSet initItem(ImExPlugin &plugin) {
    foeCreatePlugin(plugin.path.c_str(), &plugin.plugin);
    if (plugin.plugin == FOE_NULL_HANDLE) {
        return to_foeResult(FOE_SKUNKWORKS_FAILED_TO_LOAD_PLUGIN);
    }

    foeResultSet result = {.value = FOE_SUCCESS, .toString = NULL};
    for (auto const &it : plugin.initFn) {
        foeResultSet (*pFn)() = (foeResultSet (*)())foeGetPluginSymbol(plugin.plugin, it.c_str());
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

extern "C" foeResultSet registerBasicFunctionality() {
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

extern "C" void deregisterBasicFunctionality() {
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