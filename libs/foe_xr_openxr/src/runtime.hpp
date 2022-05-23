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