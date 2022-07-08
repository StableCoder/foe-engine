// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_REGISTRATION_H
#define FOE_GRAPHICS_RESOURCE_REGISTRATION_H

#include <foe/error_code.h>
#include <foe/graphics/resource/export.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_GFX_RES_EXPORT int foeGraphicsResourceFunctionalityID();

FOE_GFX_RES_EXPORT foeResult foeGraphicsResourceRegisterFunctionality();

FOE_GFX_RES_EXPORT void foeGraphicsResourceDeregisterFunctionality();

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_RESOURCE_REGISTRATION_H