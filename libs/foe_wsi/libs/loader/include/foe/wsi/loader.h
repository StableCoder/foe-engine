// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_WSI_LOADER_HPP
#define FOE_WSI_LOADER_HPP

#include <foe/wsi/export.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_WSI_EXPORT
bool foeWsiLoadedImplementation();

FOE_WSI_EXPORT
bool foeWsiLoadImplementation(char const *pPath);

#ifdef __cplusplus
}
#endif

#endif // FOE_WSI_LOADER_HPP