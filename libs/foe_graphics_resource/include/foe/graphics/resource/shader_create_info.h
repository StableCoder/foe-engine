// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_SHADER_CREATE_INFO_H
#define FOE_GRAPHICS_RESOURCE_SHADER_CREATE_INFO_H

#include <foe/graphics/vk/shader.h>
#include <foe/resource/create_info.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct foeShaderCreateInfo {
    char const *pFile;
    foeGfxVkShaderCreateInfo gfxCreateInfo;
} foeShaderCreateInfo;

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_RESOURCE_SHADER_CREATE_INFO_H