// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_VERTEX_DESCRIPTOR_HPP
#define FOE_GRAPHICS_RESOURCE_VERTEX_DESCRIPTOR_HPP

#include <foe/graphics/resource/export.h>
#include <foe/graphics/vk/vertex_descriptor.h>
#include <foe/resource/resource.h>

struct foeVertexDescriptor {
    foeResource vertexShader;
    foeResource tessellationControlShader;
    foeResource tessellationEvaluationShader;
    foeResource geometryShader;

    foeGfxVkVertexDescriptor vertexDescriptor;
};

FOE_GFX_RES_EXPORT void cleanup_foeVertexDescriptor(foeVertexDescriptor *pData);

#endif // FOE_GRAPHICS_RESOURCE_VERTEX_DESCRIPTOR_HPP