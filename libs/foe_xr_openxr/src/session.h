// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef SESSION_H
#define SESSION_H

#include <foe/xr/openxr/session.h>
#include <openxr/openxr.h>

#ifdef __cplusplus
extern "C" {
#endif

struct foeOpenXrSession {
    // From the runtime
    foeXrRuntime runtime;

    XrSession session;
    // For the session
    XrSystemId systemId;
    XrSessionState state;
    /// This bool determines if we should be doing the wait/begin/end frame calls
    bool active{false};
    XrViewConfigurationType type;
    XrSpace space;
};

FOE_DEFINE_HANDLE_CASTS(session, foeOpenXrSession, foeXrSession)

#ifdef __cplusplus
}
#endif

#endif // SESSION_H