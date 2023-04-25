// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_XR_RUNTIME_H
#define FOE_XR_RUNTIME_H

#include <foe/handle.h>

#ifdef FOE_XR_SUPPORT
#include <foe/result.h>
#include <foe/xr/export.h>
#endif // FOE_XR_SUPPORT

#ifdef __cplusplus
extern "C" {
#endif

/** Handle definition for foeXrRuntime
 *
 * Not all platforms support XR, espeically the Apple ecosystem. However, we still need the XR
 * runtime type to pass through to some other systems, notably Graphics.
 *
 * As such, this handle type can be used for passing around and compiling on non-XR supported
 * systems, without having to have two separate ABIs for other libraries, but to limit dummy stubs
 * or fancy export work, no functions are actually exported by this library.
 */
FOE_DEFINE_HANDLE(foeXrRuntime)

#ifdef FOE_XR_SUPPORT
FOE_XR_EXPORT
foeResultSet foeXrDestroyRuntime(foeXrRuntime runtime);
#endif // FOE_XR_SUPPORT

#ifdef __cplusplus
}
#endif

#endif // FOE_XR_RUNTIME_H