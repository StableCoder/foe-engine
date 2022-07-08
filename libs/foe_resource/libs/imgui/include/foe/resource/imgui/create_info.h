// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_RESOURCE_IMGUI_CREATE_INFO_H
#define FOE_RESOURCE_IMGUI_CREATE_INFO_H

#include <foe/resource/create_info.h>
#include <foe/resource/imgui/export.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_RES_IMGUI_EXPORT void imgui_foeResourceCreateInfo(foeResourceCreateInfo createInfo);

#ifdef __cplusplus
}
#endif

#endif // FOE_RESOURCE_IMGUI_CREATE_INFO_H