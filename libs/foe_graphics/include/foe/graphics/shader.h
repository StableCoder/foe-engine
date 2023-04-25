// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_SHADER_H
#define FOE_GRAPHICS_SHADER_H

#include <foe/graphics/builtin_descriptor_sets.h>
#include <foe/graphics/export.h>
#include <foe/graphics/session.h>
#include <foe/handle.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeGfxShader)

FOE_GFX_EXPORT
void foeGfxDestroyShader(foeGfxSession session, foeGfxShader shader);

FOE_GFX_EXPORT
foeBuiltinDescriptorSetLayoutFlags foeGfxShaderGetBuiltinDescriptorSetLayouts(foeGfxShader shader);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_SHADER_H