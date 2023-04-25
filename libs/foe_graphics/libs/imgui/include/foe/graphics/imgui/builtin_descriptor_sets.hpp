// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_IMGUI_BUILTIN_DESCRIPTOR_SETS_HPP
#define FOE_GRAPHICS_IMGUI_BUILTIN_DESCRIPTOR_SETS_HPP

#include <foe/graphics/builtin_descriptor_sets.h>
#include <foe/graphics/imgui/export.h>

#include <string>

FOE_GFX_IMGUI_EXPORT
void imgui_foeBuiltinDescriptorSetLayoutFlags(std::string const &label,
                                              foeBuiltinDescriptorSetLayoutFlags const &data);

#endif // FOE_GRAPHICS_IMGUI_BUILTIN_DESCRIPTOR_SETS_HPP