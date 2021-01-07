/*
    Copyright (C) 2020 George Cave.

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

#include <foe/xr/runtime.hpp>

#include <foe/xr/core.hpp>
#include <foe/xr/debug_utils.hpp>
#include <foe/xr/error_code.hpp>

std::error_code foeXrRuntime::createRuntime(char const *appName,
                                            uint32_t appVersion,
                                            std::vector<std::string> const &apiLayers,
                                            std::vector<std::string> const &extensions,
                                            bool debugLogging) {
    XrResult xrRes = foeXrCreateInstance("FoE Engine", 0, apiLayers, extensions, &instance);
    if (xrRes != XR_SUCCESS) {
        return xrRes;
    }

    if (debugLogging) {
        xrRes = foeXrCreateDebugUtilsMessenger(instance, &debugMessenger);
    }

    return xrRes;
}

void foeXrRuntime::destroyRuntime() {
    if (debugMessenger != XR_NULL_HANDLE) {
        foeXrDestroyDebugUtilsMessenger(instance, debugMessenger);
    }
    debugMessenger = XR_NULL_HANDLE;

    if (instance != XR_NULL_HANDLE) {
        xrDestroyInstance(instance);
    }
    instance = XR_NULL_HANDLE;
}

std::error_code foeXrRuntime::pollEvent(XrEventDataBuffer &event) {
    event = {
        .type = XR_TYPE_EVENT_DATA_BUFFER,
    };

    return xrPollEvent(instance, &event);
}