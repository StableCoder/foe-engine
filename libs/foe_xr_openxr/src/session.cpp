// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/xr/openxr/session.hpp>

#include <foe/xr/openxr/core.hpp>

#include "result.h"
#include "runtime.hpp"
#include "xr_result.h"

foeResultSet foeOpenXrSession::createSession(foeXrRuntime runtime,
                                             XrSystemId systemId,
                                             XrViewConfigurationType configType,
                                             void const *pGraphicsBinding) {
    auto *pRuntime = runtime_from_handle(runtime);

    XrSessionCreateInfo sessionCI{
        .type = XR_TYPE_SESSION_CREATE_INFO,
        .next = pGraphicsBinding,
        .systemId = systemId,
    };
    XrResult xrResult = xrCreateSession(pRuntime->instance, &sessionCI, &session);
    if (xrResult != XR_SUCCESS) {
        return xr_to_foeResult(xrResult);
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
    foeResultSet result = foeOpenXrEnumerateReferenceSpaces(session, refSpaces);
    if (result.value != FOE_SUCCESS) {
        return result;
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
    xrResult = xrCreateReferenceSpace(session, &refSpaceCI, &space);
    if (xrResult != XR_SUCCESS) {
        return xr_to_foeResult(xrResult);
    }

    return to_foeResult(FOE_OPENXR_SUCCESS);
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

foeResultSet foeOpenXrSession::beginSession() {
    XrSessionBeginInfo sessionBI{
        .type = XR_TYPE_SESSION_BEGIN_INFO,
        .primaryViewConfigurationType = type,
    };
    XrResult xrResult = xrBeginSession(session, &sessionBI);

    if (xrResult == XR_SUCCESS) {
        // We've got everything readied and are beginning the active state, so to sycnhronize it
        // will start calling upon the wait/begin/end frame functions
        active = true;
    }

    return xr_to_foeResult(XR_SUCCESS);
}

foeResultSet foeOpenXrSession::requestExitSession() {
    return xr_to_foeResult(xrRequestExitSession(session));
}

foeResultSet foeOpenXrSession::endSession() { return xr_to_foeResult(xrEndSession(session)); }