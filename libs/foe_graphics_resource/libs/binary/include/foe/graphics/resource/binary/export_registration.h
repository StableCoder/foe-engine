// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_BINARY_EXPORT_REGISTRATION_HPP
#define FOE_GRAPHICS_RESOURCE_BINARY_EXPORT_REGISTRATION_HPP

#include <foe/graphics/resource/binary/export.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_GFX_RES_BINARY_EXPORT
foeResultSet foeGraphicsResourceBinaryRegisterExporters();

FOE_GFX_RES_BINARY_EXPORT
void foeGraphicsResourceBinaryDeregisterExporters();

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_RESOURCE_BINARY_EXPORT_REGISTRATION_HPP