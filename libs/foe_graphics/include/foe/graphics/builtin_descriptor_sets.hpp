/*
    Copyright (C) 2020 George Cave.

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

#ifndef FOE_GRAPHICS_BUILTIN_DESCRIPTOR_SETS_HPP
#define FOE_GRAPHICS_BUILTIN_DESCRIPTOR_SETS_HPP

#include <foe/graphics/export.h>

#include <array>
#include <string>
#include <string_view>

enum foeBuiltinDescriptorSetLayoutFlagBits {
    FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX = 0x00000001,
    FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_MODEL_MATRIX = 0x00000002,
    FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_BONE_STATE_MATRICES = 0x00000004,
};
using foeBuiltinDescriptorSetLayoutFlags = uint32_t;

FOE_GFX_EXPORT std::string to_string(foeBuiltinDescriptorSetLayoutFlagBits builtinSetLayout);
FOE_GFX_EXPORT foeBuiltinDescriptorSetLayoutFlagBits to_builtin_set_layout(std::string_view str);

enum foeDescriptorSetLayoutIndex {
    ProjectionViewMatrix = 0,
    ModelMatrix = 1,
    ModelAndBoneStateMatrices = 2,
    VertexShader = 3,
    TessellationControlShader = 4,
    TessellationEvaluationShader = 5,
    GeometryShader = 6,
    FragmentShader = 7,
};

#endif // FOE_GRAPHICS_BUILTIN_DESCRIPTOR_SETS_HPP