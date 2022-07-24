// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_XR_OPENXR_SESSION_HPP
#define FOE_XR_OPENXR_SESSION_HPP

#include <foe/result.h>
#include <foe/xr/export.h>
#include <foe/xr/runtime.h>
#include <openxr/openxr.h>

struct FOE_XR_EXPORT foeOpenXrSession {
    foeResultSet createSession(foeXrRuntime runtime,
                               XrSystemId systemId,
                               XrViewConfigurationType configType,
                               void const *pGraphicsBinding);
    void destroySession();

    foeResultSet beginSession();
    foeResultSet requestExitSession();
    foeResultSet endSession();

    // From the runtime
    foeXrRuntime runtime;

    // For the session
    XrSystemId systemId;
    XrSession session;
    XrSessionState state;
    /// This bool determines if we should be doing the wait/begin/end frame calls
    bool active{false};
    XrViewConfigurationType type;
    XrSpace space;
};

#endif // FOE_XR_OPENXR_SESSION_HPP