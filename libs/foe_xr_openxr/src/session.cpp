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

#include <foe/xr/session.hpp>

#include <foe/xr/core.hpp>
#include <foe/xr/error_code.hpp>

#include "runtime.hpp"

std::error_code foeXrSession::createSession(foeXrRuntime runtime,
                                            XrSystemId systemId,
                                            XrViewConfigurationType configType,
                                            void const *pGraphicsBinding) {
    auto *pRuntime = runtime_from_handle(runtime);

    XrSessionCreateInfo sessionCI{
        .type = XR_TYPE_SESSION_CREATE_INFO,
        .next = pGraphicsBinding,
        .systemId = systemId,
    };
    XrResult xrRes = xrCreateSession(pRuntime->instance, &sessionCI, &session);
    if (xrRes != XR_SUCCESS) {
        return xrRes;
    } else {
        // Add the new session to the runtime
        foeXrOpenAddSessionToRuntime(pRuntime, this);
    }

    this->runtime = runtime;
    state = XR_SESSION_STATE_IDLE;
    type = configType;
    this->systemId = systemId;

    // Reference Space
    std::vector<XrReferenceSpaceType> refSpaces;
    xrRes = foeXrEnumerateReferenceSpaces(session, refSpaces);
    if (xrRes != XR_SUCCESS) {
        return xrRes;
    }

    XrPosef identity{
        .orientation = {.x = 0, .y = 0, .z = 0, .w = 1.0},
        .position = {.x = 0, .y = 0, .z = 0},
    };

    XrReferenceSpaceCreateInfo refSpaceCI{
        .type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO,
        .referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL,
        .poseInReferenceSpace = identity,
    };
    xrRes = xrCreateReferenceSpace(session, &refSpaceCI, &space);
    if (xrRes != XR_SUCCESS) {
        return xrRes;
    }

    return xrRes;
}

void foeXrSession::destroySession() {
    if (space != XR_NULL_HANDLE) {
        xrDestroySpace(space);
    }
    space = XR_NULL_HANDLE;

    if (session != XR_NULL_HANDLE) {
        xrDestroySession(session);

        // Remove the session from the runtime
        foeXrOpenAddSessionToRuntime(runtime_from_handle(runtime), this);
    }
    session = XR_NULL_HANDLE;
}

std::error_code foeXrSession::beginSession() {
    XrSessionBeginInfo sessionBI{
        .type = XR_TYPE_SESSION_BEGIN_INFO,
        .primaryViewConfigurationType = type,
    };
    return xrBeginSession(session, &sessionBI);
}

std::error_code foeXrSession::requestExitSession() { return xrRequestExitSession(session); }

std::error_code foeXrSession::endSession() { return xrEndSession(session); }