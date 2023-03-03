// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_MATERIAL_HPP
#define FOE_GRAPHICS_RESOURCE_MATERIAL_HPP

#include <foe/graphics/resource/export.h>
#include <foe/resource/resource.h>
#include <foe/resource/type_defs.h>
#include <vulkan/vulkan.h>

struct foeGfxVkFragmentDescriptor;

struct foeMaterial {
    foeResourceType rType;
    void *pNext;
    // For the FragmentDescriptor
    foeResource fragmentShader;
    // For the Material
    foeResource image;
    // Managed by material loader
    VkDescriptorSet materialDescriptorSet;

    // Pointer to data managed by external system (FragmentDescriptorPool)
    foeGfxVkFragmentDescriptor *pGfxFragDescriptor;
};

#endif // FOE_GRAPHICS_RESOURCE_MATERIAL_HPP