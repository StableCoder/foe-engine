// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef XR_HPP
#define XR_HPP

#include <foe/error_code.h>
#include <foe/graphics/session.h>
#include <foe/xr/openxr/session.hpp>
#include <foe/xr/runtime.h>

foeResult createXrRuntime(bool debugLogging, foeXrRuntime *pRuntime);

foeResult createXrSession(foeXrRuntime runtime,
                          foeGfxSession gfxSession,
                          foeOpenXrSession *pSession);

#endif // XR_HPP