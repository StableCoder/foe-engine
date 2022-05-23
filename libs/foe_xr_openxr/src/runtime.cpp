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

#include <foe/xr/openxr/runtime.hpp>

#include <foe/delimited_string.h>
#include <foe/engine_detail.h>
#include <foe/xr/openxr/core.hpp>
#include <foe/xr/openxr/session.hpp>

#include "debug_utils.hpp"
#include "log.hpp"
#include "result.h"
#include "runtime.hpp"
#include "xr_result.h"

foeResult foeOpenXrCreateRuntime(char const *appName,
                                 uint32_t appVersion,
                                 uint32_t layerCount,
                                 char const *const *ppLayerNames,
                                 uint32_t extensionCount,
                                 char const *const *ppExtensionNames,
                                 bool validation,
                                 bool debugLogging,
                                 foeXrRuntime *pRuntime) {
    auto *pNewRuntime = new foeOpenXrRuntime;
    XrResult xrRes{XR_SUCCESS};

    // Application Info
    pNewRuntime->apiVersion = XR_MAKE_VERSION(1, 0, 12);

    XrApplicationInfo appInfo{
        .applicationVersion = appVersion,
        .engineVersion = FOE_ENGINE_VERSION,
        .apiVersion = pNewRuntime->apiVersion,
    };
    strncpy(appInfo.applicationName, appName, XR_MAX_APPLICATION_NAME_SIZE);
    strncpy(appInfo.engineName, FOE_ENGINE_NAME, XR_MAX_ENGINE_NAME_SIZE);

    // Layers / Extensions
    std::vector<char const *> layers;
    std::vector<char const *> extensions;

    for (uint32_t i = 0; i < layerCount; ++i)
        layers.emplace_back(ppLayerNames[i]);
    for (uint32_t i = 0; i < extensionCount; ++i)
        extensions.emplace_back(ppExtensionNames[i]);

    if (validation) {
        layers.emplace_back("XR_APILAYER_LUNARG_core_validation");
        FOE_LOG(foeOpenXr, Verbose, "Adding validation layers to new XrInstance");
    }
    if (debugLogging) {
        extensions.emplace_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);
        FOE_LOG(foeOpenXr, Verbose, "Adding debug report extension to new XrInstance");
    }

    // Create Instance
    XrInstanceCreateInfo instanceCI = {
        .type = XR_TYPE_INSTANCE_CREATE_INFO,
        .applicationInfo = appInfo,
        .enabledApiLayerCount = static_cast<uint32_t>(layers.size()),
        .enabledApiLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .enabledExtensionNames = extensions.data(),
    };

    xrRes = xrCreateInstance(&instanceCI, &pNewRuntime->instance);
    if (xrRes != XR_SUCCESS)
        goto CREATE_FAILED;

    // Add layer/extension state to runtime struct for future queries
    foeCreateDelimitedString(layers.size(), layers.data(), &pNewRuntime->layerNamesLength, nullptr);
    if (pNewRuntime->layerNamesLength != 0) {
        pNewRuntime->pLayerNames = new char[pNewRuntime->layerNamesLength];
        foeCreateDelimitedString(layers.size(), layers.data(), &pNewRuntime->layerNamesLength,
                                 pNewRuntime->pLayerNames);
    }

    foeCreateDelimitedString(extensions.size(), extensions.data(),
                             &pNewRuntime->extensionNamesLength, nullptr);
    if (pNewRuntime->extensionNamesLength != 0) {
        pNewRuntime->pExtensionNames = new char[pNewRuntime->extensionNamesLength];
        foeCreateDelimitedString(extensions.size(), extensions.data(),
                                 &pNewRuntime->extensionNamesLength, pNewRuntime->pExtensionNames);
    }

    // Attach Debug Logger
    if (debugLogging) {
        xrRes =
            foeOpenXrCreateDebugUtilsMessenger(pNewRuntime->instance, &pNewRuntime->debugMessenger);
        if (xrRes != XR_SUCCESS)
            goto CREATE_FAILED;

        FOE_LOG(foeOpenXr, Verbose, "Added debug logging to new XrInstance");
    }

CREATE_FAILED:
    if (xrRes != XR_SUCCESS) {
        foeXrDestroyRuntime(runtime_to_handle(pNewRuntime));
    } else {
        *pRuntime = runtime_to_handle(pNewRuntime);
    }

    return xr_to_foeResult(xrRes);
}

foeResult foeOpenXrEnumerateRuntimeVersion(foeXrRuntime runtime, uint32_t *pApiVersion) {
    auto *pRuntime = runtime_from_handle(runtime);

    *pApiVersion = pRuntime->apiVersion;

    return to_foeResult(FOE_OPENXR_SUCCESS);
}

foeResult foeOpenXrEnumerateRuntimeLayers(foeXrRuntime runtime,
                                          uint32_t *pLayerNamesLength,
                                          char *pLayerNames) {
    auto *pRuntime = runtime_from_handle(runtime);

    return foeCopyDelimitedString(pRuntime->layerNamesLength, pRuntime->pLayerNames,
                                  pLayerNamesLength, pLayerNames)
               ? to_foeResult(FOE_OPENXR_SUCCESS)
               : to_foeResult(FOE_OPENXR_INCOMPLETE);
}

foeResult foeOpenXrEnumerateRuntimeExtensions(foeXrRuntime runtime,
                                              uint32_t *pExtensionNamesLength,
                                              char *pExtensionNames) {
    auto *pRuntime = runtime_from_handle(runtime);

    return foeCopyDelimitedString(pRuntime->extensionNamesLength, pRuntime->pExtensionNames,
                                  pExtensionNamesLength, pExtensionNames)
               ? to_foeResult(FOE_OPENXR_SUCCESS)
               : to_foeResult(FOE_OPENXR_INCOMPLETE);
}

foeResult foeXrProcessEvents(foeXrRuntime runtime) {
    auto *pRuntime = runtime_from_handle(runtime);

    XrEventDataBuffer event = {
        .type = XR_TYPE_EVENT_DATA_BUFFER,
    };

    XrResult xrResult = xrPollEvent(pRuntime->instance, &event);

    if (xrResult == XR_EVENT_UNAVAILABLE) {
        // No event
        return xr_to_foeResult(XR_SUCCESS);
    } else if (xrResult) {
        // Some other error occurred
        return xr_to_foeResult(xrResult);
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
            FOE_LOG(foeOpenXr, Warning, "Unprocessed XR event!!!");
            break;
        }
    }

    return to_foeResult(FOE_OPENXR_SUCCESS);
}

XrInstance foeOpenXrGetInstance(foeXrRuntime runtime) {
    auto *pRuntime = runtime_from_handle(runtime);
    return pRuntime->instance;
}

foeResult foeXrDestroyRuntime(foeXrRuntime runtime) {
    auto *pRuntime = runtime_from_handle(runtime);
    XrResult xrResult = XR_SUCCESS;

    if (pRuntime->debugMessenger != XR_NULL_HANDLE)
        foeOpenXrDestroyDebugUtilsMessenger(pRuntime->instance, pRuntime->debugMessenger);

    if (pRuntime->instance != XR_NULL_HANDLE)
        xrResult = xrDestroyInstance(pRuntime->instance);

    delete pRuntime;

    return xr_to_foeResult(xrResult);
}

void foeOpenXrAddSessionToRuntime(foeOpenXrRuntime *pRuntime, foeOpenXrSession *pSession) {
    std::scoped_lock lock{pRuntime->sync};
    pRuntime->sessions.emplace_back(pSession);
}

void foeOpenXrRemoveSessionFromRuntime(foeOpenXrRuntime *pRuntime, foeOpenXrSession *pSession) {
    std::scoped_lock lock{pRuntime->sync};
    for (auto it = pRuntime->sessions.begin(); it != pRuntime->sessions.end(); ++it) {
        if (*it == pSession) {
            pRuntime->sessions.erase(it);
            break;
        }
    }
}