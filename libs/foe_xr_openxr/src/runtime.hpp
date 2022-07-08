// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include <foe/xr/runtime.h>
#include <openxr/openxr.h>

#include <mutex>
#include <vector>

struct foeOpenXrSession;

struct foeOpenXrRuntime {
    std::mutex sync;

    XrInstance instance{XR_NULL_HANDLE};
    XrDebugUtilsMessengerEXT debugMessenger{XR_NULL_HANDLE};

    std::vector<foeOpenXrSession *> sessions;

    /// The XrInstance API version created with
    XrVersion apiVersion;

    /// Length in bytes of pLayerNames
    uint32_t layerNamesLength;
    /// Set of strings representing the instance's layers, delimited by NULL characters
    char *pLayerNames;
    /// Length in bytes of pExtensionNames
    uint32_t extensionNamesLength;
    /// Set of strings representing the instance's extensions, delimited by NULL characters
    char *pExtensionNames;
};

FOE_DEFINE_HANDLE_CASTS(runtime, foeOpenXrRuntime, foeXrRuntime)

void foeOpenXrAddSessionToRuntime(foeOpenXrRuntime *pRuntime, foeOpenXrSession *pSession);

void foeOpenXrRemoveSessionFromRuntime(foeOpenXrRuntime *pRuntime, foeOpenXrSession *pSession);

#endif // RUNTIME_HPP