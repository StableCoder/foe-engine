// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/xr/openxr/session.h>

#include "result.h"
#include "runtime.hpp"
#include "session.h"
#include "xr_result.h"

namespace {

foeResultSet foeOpenXrEnumerateReferenceSpaces(XrSession xrSession,
                                               std::vector<XrReferenceSpaceType> &spaces) {
    uint32_t spaceCount;
    XrResult xrResult = xrEnumerateReferenceSpaces(xrSession, 0, &spaceCount, nullptr);
    if (xrResult != XR_SUCCESS) {
        return xr_to_foeResult(xrResult);
    }

    spaces.resize(spaceCount);
    xrResult = xrEnumerateReferenceSpaces(xrSession, static_cast<uint32_t>(spaces.size()),
                                          &spaceCount, spaces.data());

    return xr_to_foeResult(xrResult);
}

} // namespace

extern "C" foeResultSet foeOpenXrCreateSession(foeXrRuntime runtime,
                                               XrSystemId systemId,
                                               XrViewConfigurationType configType,
                                               void const *pGraphicsBinding,
                                               foeXrSession *pSession) {
    auto *pRuntime = runtime_from_handle(runtime);

    XrSession session = XR_NULL_HANDLE;
    XrSessionCreateInfo sessionCI{
        .type = XR_TYPE_SESSION_CREATE_INFO,
        .next = pGraphicsBinding,
        .systemId = systemId,
    };

    XrResult xrResult = xrCreateSession(pRuntime->instance, &sessionCI, &session);
    if (xrResult != XR_SUCCESS)
        return xr_to_foeResult(xrResult);

    foeOpenXrSession *pNewSession = new (std::nothrow) foeOpenXrSession{
        .runtime = runtime,
        .session = session,
        .systemId = systemId,
        .state = XR_SESSION_STATE_IDLE,
        .active = false,
        .type = configType,
    };
    if (pNewSession == nullptr)
        return to_foeResult(FOE_OPENXR_ERROR_OUT_OF_MEMORY);

    foeOpenXrAddSessionToRuntime(pRuntime, pNewSession);

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

    xrResult = xrCreateReferenceSpace(session, &refSpaceCI, &pNewSession->space);
    if (xrResult != XR_SUCCESS) {
        goto CREATE_FAILED;
    }

CREATE_FAILED:
    if (xrResult != XR_SUCCESS) {
        foeXrDestroySession(session_to_handle(pNewSession));
    } else {
        *pSession = session_to_handle(pNewSession);
    }

    return to_foeResult(FOE_OPENXR_SUCCESS);
}

extern "C" void foeXrDestroySession(foeXrSession session) {
    foeOpenXrSession *pSession = session_from_handle(session);

    if (pSession->space != XR_NULL_HANDLE) {
        xrDestroySpace(pSession->space);
    }

    if (pSession->session != XR_NULL_HANDLE) {
        xrDestroySession(pSession->session);

        // Remove the session from the runtime
        foeOpenXrAddSessionToRuntime(runtime_from_handle(pSession->runtime), pSession);
    }

    delete pSession;
}

extern "C" foeResultSet foeOpenXrBeginSession(foeXrSession session) {
    foeOpenXrSession *pSession = session_from_handle(session);

    XrSessionBeginInfo sessionBI{
        .type = XR_TYPE_SESSION_BEGIN_INFO,
        .primaryViewConfigurationType = pSession->type,
    };
    XrResult xrResult = xrBeginSession(pSession->session, &sessionBI);

    if (xrResult == XR_SUCCESS) {
        // We've got everything readied and are beginning the active state, so to sycnhronize it
        // will start calling upon the wait/begin/end frame functions
        pSession->active = true;
    }

    return xr_to_foeResult(XR_SUCCESS);
}

extern "C" foeResultSet foeOpenXrEndSession(foeXrSession session) {
    foeOpenXrSession *pSession = session_from_handle(session);

    return xr_to_foeResult(xrEndSession(pSession->session));
}

extern "C" foeResultSet foeOpenXrRequestExitSession(foeXrSession session) {
    foeOpenXrSession *pSession = session_from_handle(session);

    return xr_to_foeResult(xrRequestExitSession(pSession->session));
}

extern "C" XrSession foeOpenXrGetSession(foeXrSession session) {
    foeOpenXrSession *pSession = session_from_handle(session);

    return pSession->session;
}

extern "C" XrSystemId foeOpenXrGetSystemId(foeXrSession session) {
    foeOpenXrSession *pSession = session_from_handle(session);

    return pSession->systemId;
}

extern "C" XrViewConfigurationType foeOpenXrGetViewConfigurationType(foeXrSession session) {
    foeOpenXrSession *pSession = session_from_handle(session);

    return pSession->type;
}

extern "C" XrSessionState foeOpenXrGetSessionState(foeXrSession session) {
    foeOpenXrSession *pSession = session_from_handle(session);

    return pSession->state;
}

extern "C" bool foeOpenXrGetSessionActive(foeXrSession session) {
    foeOpenXrSession *pSession = session_from_handle(session);

    return pSession->active;
}

extern "C" XrSpace foeOpenXrGetSpace(foeXrSession session) {
    foeOpenXrSession *pSession = session_from_handle(session);

    return pSession->space;
}

extern "C" foeResultSet foeOpenXrEnumerateSwapchainFormats(foeXrSession session,
                                                           uint32_t *pFormatCount,
                                                           int64_t *pFormats) {
    foeOpenXrSession *pSession = session_from_handle(session);

    XrResult xrResult;
    if (pFormats == NULL)
        xrResult = xrEnumerateSwapchainFormats(pSession->session, 0, pFormatCount, NULL);
    else
        xrResult =
            xrEnumerateSwapchainFormats(pSession->session, *pFormatCount, pFormatCount, pFormats);
    return xr_to_foeResult(xrResult);
}