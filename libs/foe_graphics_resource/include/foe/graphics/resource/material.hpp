// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_MATERIAL_HPP
#define FOE_GRAPHICS_RESOURCE_MATERIAL_HPP

#include <foe/graphics/resource/export.h>
#include <foe/resource/resource.h>
#include <vulkan/vulkan.h>

struct foeGfxVkFragmentDescriptor;

struct foeMaterial {
    // For the FragmentDescriptor
    foeResource fragmentShader;
    // For the Material
    foeResource image;

    foeGfxVkFragmentDescriptor *pGfxFragDescriptor;
    VkDescriptorSet materialDescriptorSet;
};

#endif // FOE_GRAPHICS_RESOURCE_MATERIAL_HPP