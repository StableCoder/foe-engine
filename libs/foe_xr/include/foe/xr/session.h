// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_XR_SESSION_H
#define FOE_XR_SESSION_H

#include <foe/handle.h>

#ifdef FOE_XR_SUPPORT
#include <foe/xr/export.h>
#endif // FOE_XR_SUPPORT

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeXrSession)

#ifdef FOE_XR_SUPPORT
FOE_XR_EXPORT void foeXrDestroySession(foeXrSession session);
#endif // FOE_XR_SUPPORT

#ifdef __cplusplus
}
#endif

#endif // FOE_XR_SESSION_H