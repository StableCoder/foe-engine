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

#include <foe/xr/openxr/runtime.hpp>

#include <foe/xr/core.hpp>
#include <foe/xr/error_code.hpp>

#include "debug_utils.hpp"
#include "log.hpp"
#include "runtime.hpp"

namespace {

void foeXrOpenDestroyRuntime(foeXrOpenRuntime *pRuntime) {
    if (pRuntime->debugMessenger != XR_NULL_HANDLE)
        foeXrDestroyDebugUtilsMessenger(pRuntime->instance, pRuntime->debugMessenger);

    if (pRuntime->instance != XR_NULL_HANDLE)
        xrDestroyInstance(pRuntime->instance);

    delete pRuntime;
}

} // namespace

std::error_code foeXrOpenCreateRuntime(char const *appName,
                                       uint32_t appVersion,
                                       std::vector<std::string> layers,
                                       std::vector<std::string> extensions,
                                       bool validation,
                                       bool debugLogging,
                                       foeXrRuntime *pRuntime) {
    auto *pNewRuntime = new foeXrOpenRuntime;

    if (validation) {
        layers.emplace_back("XR_APILAYER_LUNARG_core_validation");
        FOE_LOG(foeXrOpen, Verbose, "Adding validation layers to XrInstance");
    }
    if (debugLogging)
        extensions.emplace_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);

    XrResult xrRes =
        foeXrCreateInstance(appName, appVersion, layers, extensions, &pNewRuntime->instance);
    if (xrRes != XR_SUCCESS)
        goto CREATE_FAILED;

    if (debugLogging) {
        xrRes = foeXrCreateDebugUtilsMessenger(pNewRuntime->instance, &pNewRuntime->debugMessenger);
        if (xrRes != XR_SUCCESS)
            goto CREATE_FAILED;

        FOE_LOG(foeXrOpen, Verbose, "Added debug logging to XrInstance");
    }

CREATE_FAILED:
    if (xrRes != XR_SUCCESS) {
        foeXrOpenDestroyRuntime(pNewRuntime);
    } else {
        *pRuntime = runtime_to_handle(pNewRuntime);
    }

    return xrRes;
}

std::error_code foeXrOpenPollEvent(foeXrRuntime runtime, XrEventDataBuffer &event) {
    event = {
        .type = XR_TYPE_EVENT_DATA_BUFFER,
    };

    auto *pRuntime = runtime_from_handle(runtime);
    return xrPollEvent(pRuntime->instance, &event);
}

XrInstance foeXrOpenGetInstance(foeXrRuntime runtime) {
    auto *pRuntime = runtime_from_handle(runtime);
    return pRuntime->instance;
}

void foeXrDestroyRuntime(foeXrRuntime runtime) {
    auto *pRuntime = runtime_from_handle(runtime);
    foeXrOpenDestroyRuntime(pRuntime);
}