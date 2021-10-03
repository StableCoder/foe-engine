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

#include <foe/engine_detail.h>
#include <foe/xr/core.hpp>
#include <foe/xr/error_code.hpp>
#include <foe/xr/session.hpp>

#include "debug_utils.hpp"
#include "log.hpp"
#include "runtime.hpp"

std::error_code foeXrOpenCreateRuntime(char const *appName,
                                       uint32_t appVersion,
                                       std::vector<std::string> layers,
                                       std::vector<std::string> extensions,
                                       bool validation,
                                       bool debugLogging,
                                       foeXrRuntime *pRuntime) {
    auto *pNewRuntime = new foeXrOpenRuntime;
    XrResult xrRes{XR_SUCCESS};

    if (validation) {
        layers.emplace_back("XR_APILAYER_LUNARG_core_validation");
        FOE_LOG(foeXrOpen, Verbose, "Adding validation layers to XrInstance");
    }
    if (debugLogging)
        extensions.emplace_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);

    // Create XrInstance
    XrApplicationInfo appInfo{
        .applicationVersion = appVersion,
        .engineVersion = FOE_ENGINE_VERSION,
        .apiVersion = XR_MAKE_VERSION(1, 0, 12),
    };
    strncpy(appInfo.applicationName, appName, XR_MAX_APPLICATION_NAME_SIZE);
    strncpy(appInfo.engineName, FOE_ENGINE_NAME, XR_MAX_ENGINE_NAME_SIZE);

    std::vector<char const *> finalLayers;
    std::vector<char const *> finalExtensions;

    for (auto &it : layers) {
        finalLayers.emplace_back(it.data());
    }
    for (auto &it : extensions) {
        finalExtensions.emplace_back(it.data());
    }

    XrInstanceCreateInfo instanceCI = {
        .type = XR_TYPE_INSTANCE_CREATE_INFO,
        .applicationInfo = appInfo,
        .enabledApiLayerCount = static_cast<uint32_t>(finalLayers.size()),
        .enabledApiLayerNames = finalLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(finalExtensions.size()),
        .enabledExtensionNames = finalExtensions.data(),
    };

    xrRes = xrCreateInstance(&instanceCI, &pNewRuntime->instance);
    if (xrRes != XR_SUCCESS)
        goto CREATE_FAILED;

    // Attach Debug Logger
    if (debugLogging) {
        xrRes = foeXrCreateDebugUtilsMessenger(pNewRuntime->instance, &pNewRuntime->debugMessenger);
        if (xrRes != XR_SUCCESS)
            goto CREATE_FAILED;

        FOE_LOG(foeXrOpen, Verbose, "Added debug logging to XrInstance");
    }

CREATE_FAILED:
    if (xrRes != XR_SUCCESS) {
        foeXrDestroyRuntime(runtime_to_handle(pNewRuntime));
    } else {
        *pRuntime = runtime_to_handle(pNewRuntime);
    }

    return xrRes;
}

std::error_code foeXrProcessEvents(foeXrRuntime runtime) {
    auto *pRuntime = runtime_from_handle(runtime);

    XrEventDataBuffer event = {
        .type = XR_TYPE_EVENT_DATA_BUFFER,
    };

    std::error_code errC = xrPollEvent(pRuntime->instance, &event);

    if (errC == XR_EVENT_UNAVAILABLE) {
        // No event
        return XR_SUCCESS;
    } else if (errC) {
        // Some other error occurred
        return errC;
    } else {
        // Actual event retrieved
        switch (event.type) {
        case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
            XrEventDataSessionStateChanged const *stateChanged =
                reinterpret_cast<XrEventDataSessionStateChanged *>(&event);

            for (auto it : pRuntime->sessions) {
                if (it->session == stateChanged->session) {
                    if (stateChanged->state == XR_SESSION_STATE_STOPPING ||
                        stateChanged->state == XR_SESSION_STATE_LOSS_PENDING) {
                        // If the state has been lost or stopping, the session is no longer 'active'
                        // and should not call the wait/begin/end frame functions
                        it->active = false;
                    }

                    it->state = stateChanged->state;
                    break;
                }
            }
            break;
        }

        default:
            FOE_LOG(foeXrOpen, Warning, "Unprocessed XR event!!!");
            break;
        }
    }

    return XR_SUCCESS;
}

XrInstance foeXrOpenGetInstance(foeXrRuntime runtime) {
    auto *pRuntime = runtime_from_handle(runtime);
    return pRuntime->instance;
}

auto foeXrDestroyRuntime(foeXrRuntime runtime) -> std::error_code {
    auto *pRuntime = runtime_from_handle(runtime);
    std::error_code errC;

    if (pRuntime->debugMessenger != XR_NULL_HANDLE)
        foeXrDestroyDebugUtilsMessenger(pRuntime->instance, pRuntime->debugMessenger);

    if (pRuntime->instance != XR_NULL_HANDLE)
        errC = xrDestroyInstance(pRuntime->instance);

    delete pRuntime;

    return errC;
}

void foeXrOpenAddSessionToRuntime(foeXrOpenRuntime *pRuntime, foeXrSession *pSession) {
    std::scoped_lock lock{pRuntime->sync};
    pRuntime->sessions.emplace_back(pSession);
}

void foeXrOpenRemoveSessionFromRuntime(foeXrOpenRuntime *pRuntime, foeXrSession *pSession) {
    std::scoped_lock lock{pRuntime->sync};
    for (auto it = pRuntime->sessions.begin(); it != pRuntime->sessions.end(); ++it) {
        if (*it == pSession) {
            pRuntime->sessions.erase(it);
            break;
        }
    }
}