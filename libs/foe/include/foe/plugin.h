// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_PLUGIN_CORE_H
#define FOE_PLUGIN_CORE_H

#include <foe/export.h>
#include <foe/handle.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foePlugin)

/**
 *@brief Creates a plugin handle if the given plugin file can be loaded
 * @param pPath String representing the location of the plugin to load
 * @param[out] pPlugin Handle of the new plugin, returns FOE_NULL_HANDLE if not loaded
 */
FOE_EXPORT void foeCreatePlugin(char const *pPath, foePlugin *pPlugin);

/**
 *@brief Destroys the given plugin
 * @param plugin Handle of the plugin to be destroyed
 */
FOE_EXPORT void foeDestroyPlugin(foePlugin plugin);

/**
 *@brief Gets an exported symbol from the provided plugin
 * @param plugin Handle of the plugin to get the symbol from
 * @param pSymbol String of the name of the symbol to return
 * @return Pointer to the address of the symbol in the plugin, or nullptr if not found.
 */
FOE_EXPORT void *foeGetPluginSymbol(foePlugin plugin, char const *pSymbol);

#ifdef __cplusplus
}
#endif

#endif // FOE_PLUGIN_CORE_H