// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RENDER_STATE_HPP
#define RENDER_STATE_HPP

#include <foe/ecs/id.h>
#include <vulkan/vulkan.h>

struct foeRenderState {
    foeId vertexDescriptor;
    foeId bonedVertexDescriptor;
    foeId material;
    foeId mesh;

    // Runtime info
    VkDescriptorSet boneDescriptorSet{};
};

#endif // RENDER_STATE_HPP