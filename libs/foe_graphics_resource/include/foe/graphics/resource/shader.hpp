// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_SHADER_HPP
#define FOE_GRAPHICS_RESOURCE_SHADER_HPP

#include <foe/graphics/shader.h>
#include <foe/resource/type_defs.h>

struct foeShader {
    foeResourceType rType;
    void *pNext;
    foeGfxShader shader;
};

#endif // FOE_GRAPHICS_RESOURCE_SHADER_HPP