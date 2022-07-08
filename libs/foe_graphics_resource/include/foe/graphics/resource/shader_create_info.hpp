// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_SHADER_CREATE_INFO_HPP
#define FOE_GRAPHICS_RESOURCE_SHADER_CREATE_INFO_HPP

#include <foe/graphics/resource/export.h>
#include <foe/graphics/vk/shader.hpp>
#include <foe/resource/create_info.h>

#include <string>

struct foeShaderCreateInfo {
    std::string shaderCodeFile;
    foeGfxVkShaderCreateInfo gfxCreateInfo;
};

FOE_GFX_RES_EXPORT void foeDestroyShaderCreateInfo(foeResourceCreateInfoType type,
                                                   void *pCreateInfo);

#endif // FOE_GRAPHICS_RESOURCE_SHADER_CREATE_INFO_HPP