// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_XR_OPENXR_SESSION_H
#define FOE_XR_OPENXR_SESSION_H

#include <foe/result.h>
#include <foe/xr/export.h>
#include <foe/xr/runtime.h>
#include <foe/xr/session.h>
#include <openxr/openxr.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_XR_EXPORT
foeResultSet foeOpenXrCreateSession(foeXrRuntime runtime,
                                    XrSystemId systemId,
                                    XrViewConfigurationType configType,
                                    void const *pGraphicsBinding,
                                    foeXrSession *pSession);

FOE_XR_EXPORT
foeResultSet foeOpenXrBeginSession(foeXrSession session);

FOE_XR_EXPORT
foeResultSet foeOpenXrEndSession(foeXrSession session);

FOE_XR_EXPORT
foeResultSet foeOpenXrRequestExitSession(foeXrSession session);

FOE_XR_EXPORT
XrSession foeOpenXrGetSession(foeXrSession session);
FOE_XR_EXPORT
XrSystemId foeOpenXrGetSystemId(foeXrSession session);
FOE_XR_EXPORT
XrViewConfigurationType foeOpenXrGetViewConfigurationType(foeXrSession session);
FOE_XR_EXPORT
XrSessionState foeOpenXrGetSessionState(foeXrSession session);
FOE_XR_EXPORT
bool foeOpenXrGetSessionActive(foeXrSession session);
FOE_XR_EXPORT
XrSpace foeOpenXrGetSpace(foeXrSession session);

FOE_XR_EXPORT
foeResultSet foeOpenXrEnumerateSwapchainFormats(foeXrSession session,
                                                uint32_t *pFormatCount,
                                                int64_t *pFormats);

#ifdef __cplusplus
}
#endif

#endif // FOE_XR_OPENXR_SESSION_H