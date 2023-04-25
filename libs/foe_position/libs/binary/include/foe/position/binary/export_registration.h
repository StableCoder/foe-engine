// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_POSITION_BINARY_EXPORT_REGISTRATION_HPP
#define FOE_POSITION_BINARY_EXPORT_REGISTRATION_HPP

#include <foe/position/binary/export.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_POSITION_BINARY_EXPORT
foeResultSet foePositionBinaryRegisterExporters();

FOE_POSITION_BINARY_EXPORT
void foePositionBinaryDeregisterExporters();

#ifdef __cplusplus
}
#endif

#endif // FOE_POSITION_BINARY_EXPORT_REGISTRATION_HPP