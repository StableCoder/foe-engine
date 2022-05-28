/*
    Copyright (C) 2020-2022 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

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

FOE_GFX_EXPORT void foeGfxDestroyShader(foeGfxSession session, foeGfxShader shader);

FOE_GFX_EXPORT foeBuiltinDescriptorSetLayoutFlags
foeGfxShaderGetBuiltinDescriptorSetLayouts(foeGfxShader shader);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_SHADER_H