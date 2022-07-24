// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_XR_OPENXR_RUNTIME_HPP
#define FOE_XR_OPENXR_RUNTIME_HPP

#include <foe/error_code.h>
#include <foe/xr/export.h>
#include <foe/xr/runtime.h>
#include <openxr/openxr.h>

FOE_XR_EXPORT foeResultSet foeOpenXrCreateRuntime(char const *appName,
                                                  uint32_t appVersion,
                                                  uint32_t layerCount,
                                                  char const *const *ppLayerNames,
                                                  uint32_t extensionCount,
                                                  char const *const *ppExtensionNames,
                                                  bool validation,
                                                  bool debugLogging,
                                                  foeXrRuntime *pRuntime);

/** @brief Queries the XrInstance-level functionality the runtime was created with
 * @param runtime is the handle to the runtime to query
 * @param pApiVersion is a pointer to a XrVersion, which is the version of OpenXR the runtime was
 * created with.
 * @return FOE_OPENXR_SUCCESS
 */
FOE_XR_EXPORT foeResultSet foeOpenXrEnumerateRuntimeVersion(foeXrRuntime runtime,
                                                            XrVersion *pApiVersion);

/** @brief Enumerate the enabled layers for the given runtime
 * @param runtime is the handle to the runtime whose layers will be queried.
 * @param pLayerNamesLength is a pointer to an integer related to the size of pLayarNames, as
 * described below.
 * @param pLayerNames is either NULL or a pointer to an character array.
 * @return FOE_OPENXR_SUCCESS on success, FOE_OPENXR_INCOMPLETE if not all names could be returned
 *
 * If pLayerNames is NULL, then the size required to return all layer names is returned int
 * pLayerNamesLength. Otherwise, pLayerNamesLength must point to a variable set by the user to the
 * size of the pLayerNames array, and on return the variable is overwritten with the characters
 * actually written to pLayerNames. If pLayerNamesLength is less than the total size required to
 * return all names, at most pLayerNamesLength is written, and FOE_GFX_VK_INCOMPLETE will be
 * returned instead of FOE_GFX_VK_SUCCESS, to indicate that not all names were returned.
 */
FOE_XR_EXPORT foeResultSet foeOpenXrEnumerateRuntimeLayers(foeXrRuntime runtime,
                                                           uint32_t *pLayerNamesLength,
                                                           char *pLayerNames);

/** @brief Enumerate the enabled extensions for the given runtime
 * @param runtime is the handle to the runtime whose extensions will be queried.
 * @param pExtensionNamesLength is a pointer to an integer related to the size of pExtensionNames,
 * as described below.
 * @param pExtensionNames is either NULL or a pointer to an character array.
 * @return FOE_OPENXR_SUCCESS on success, FOE_OPENXR_INCOMPLETE if not all names could be returned
 *
 * If pExtensionNames is NULL, then the size required to return all layer names is returned int
 * pExtensionNamesLength. Otherwise, pExtensionNamesLength must point to a variable set by
 * the user to the size of the pExtensionNames array, and on return the variable is overwritten
 * with the characters actually written to pExtensionNames. If pExtensionNamesLength is
 * less than the total size required to return all names, at most pExtensionNamesLength is
 * written, and FOE_GFX_VK_INCOMPLETE will be returned instead of FOE_GFX_VK_SUCCESS, to indicate
 * that not all names were returned.
 */
FOE_XR_EXPORT foeResultSet foeOpenXrEnumerateRuntimeExtensions(foeXrRuntime runtime,
                                                               uint32_t *pExtensionNamesLength,
                                                               char *pExtensionNames);

FOE_XR_EXPORT foeResultSet foeXrDestroyRuntime(foeXrRuntime runtime);

FOE_XR_EXPORT foeResultSet foeXrProcessEvents(foeXrRuntime runtime);

#include <openxr/openxr.h>

FOE_XR_EXPORT XrInstance foeOpenXrGetInstance(foeXrRuntime runtime);

#endif // FOE_XR_OPENXR_RUNTIME_HPP