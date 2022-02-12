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

#include <foe/xr/openxr/session.hpp>

#include <foe/xr/openxr/core.hpp>
#include <foe/xr/openxr/error_code.hpp>

#include "runtime.hpp"

std::error_code foeOpenXrSession::createSession(foeXrRuntime runtime,
                                                XrSystemId systemId,
                                                XrViewConfigurationType configType,
                                                void const *pGraphicsBinding) {
    auto *pRuntime = runtime_from_handle(runtime);

    XrSessionCreateInfo sessionCI{
        .type = XR_TYPE_SESSION_CREATE_INFO,
        .next = pGraphicsBinding,
        .systemId = systemId,
    };
    std::error_code errC = xrCreateSession(pRuntime->instance, &sessionCI, &session);
    if (errC) {
        return errC;
    } else {
        // Add the new session to the runtime
        foeOpenXrAddSessionToRuntime(pRuntime, this);
    }

    this->runtime = runtime;
    state = XR_SESSION_STATE_IDLE;
    active = false;
    type = configType;
    this->systemId = systemId;

    // Reference Space
    std::vector<XrReferenceSpaceType> refSpaces;
    errC = foeOpenXrEnumerateReferenceSpaces(session, refSpaces);
    if (errC) {
        return errC;
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
    errC = xrCreateReferenceSpace(session, &refSpaceCI, &space);
    if (errC) {
        return errC;
    }

    return errC;
}

void foeOpenXrSession::destroySession() {
    if (space != XR_NULL_HANDLE) {
        xrDestroySpace(space);
    }
    space = XR_NULL_HANDLE;

    if (session != XR_NULL_HANDLE) {
        xrDestroySession(session);

        // Remove the session from the runtime
        foeOpenXrAddSessionToRuntime(runtime_from_handle(runtime), this);
    }
    session = XR_NULL_HANDLE;
}

std::error_code foeOpenXrSession::beginSession() {
    XrSessionBeginInfo sessionBI{
        .type = XR_TYPE_SESSION_BEGIN_INFO,
        .primaryViewConfigurationType = type,
    };
    auto errC = xrBeginSession(session, &sessionBI);

    if (!errC) {
        // We've got everything readied and are beginning the active state, so to sycnhronize it
        // will start calling upon the wait/begin/end frame functions
        active = true;
    }

    return errC;
}

std::error_code foeOpenXrSession::requestExitSession() { return xrRequestExitSession(session); }

std::error_code foeOpenXrSession::endSession() { return xrEndSession(session); }