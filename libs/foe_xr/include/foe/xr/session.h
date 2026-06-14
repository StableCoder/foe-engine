// Copyright (C) 2020-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_XR_SESSION_H
#define FOE_XR_SESSION_H

#include <foe/handle.h>

#ifdef FOE_SUPPORT_XR
#include <foe/xr/export.h>
#endif // FOE_SUPPORT_XR

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeXrSession)

#ifdef FOE_SUPPORT_XR
FOE_XR_EXPORT
void foeXrDestroySession(foeXrSession session);
#endif // FOE_SUPPORT_XR

#ifdef __cplusplus
}
#endif

#endif // FOE_XR_SESSION_H