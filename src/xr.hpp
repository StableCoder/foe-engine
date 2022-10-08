// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef XR_HPP
#define XR_HPP

#include <foe/graphics/session.h>
#include <foe/result.h>
#include <foe/xr/openxr/session.h>
#include <foe/xr/runtime.h>

foeResultSet createXrRuntime(bool debugLogging, foeXrRuntime *pRuntime);

foeResultSet createXrSession(foeXrRuntime runtime,
                             foeGfxSession gfxSession,
                             foeXrSession *pSession);

#endif // XR_HPP