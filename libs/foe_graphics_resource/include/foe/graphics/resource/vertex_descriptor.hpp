// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_VERTEX_DESCRIPTOR_HPP
#define FOE_GRAPHICS_RESOURCE_VERTEX_DESCRIPTOR_HPP

#include <foe/graphics/vk/vertex_descriptor.hpp>
#include <foe/resource/resource.h>

struct foeVertexDescriptor {
    foeResource vertexShader;
    foeResource tessellationControlShader;
    foeResource tessellationEvaluationShader;
    foeResource geometryShader;

    foeGfxVertexDescriptor vertexDescriptor;
};

#endif // FOE_GRAPHICS_RESOURCE_VERTEX_DESCRIPTOR_HPP