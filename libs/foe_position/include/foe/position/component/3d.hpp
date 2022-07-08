// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_POSITION_COMPONENT_3D_HPP
#define FOE_POSITION_COMPONENT_3D_HPP

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

struct foePosition3d {
    glm::vec3 position;
    glm::quat orientation;

    VkDescriptorSet descriptorSet{VK_NULL_HANDLE};
};

#endif // FOE_POSITION_COMPONENT_3D_HPP