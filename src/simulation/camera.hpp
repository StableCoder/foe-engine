// Copyright (C) 2020-2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <foe/position/component/3d.hpp>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <array>
#include <vector>

class CameraDescriptorPool;
struct foeCameraBase {
    virtual ~foeCameraBase() = default;

    virtual glm::mat4 projectionMatrix() const noexcept = 0;

    // Graphics Data
    VkDescriptorSet descriptor{VK_NULL_HANDLE};
};

struct Camera : public foeCameraBase {
    // Projection Data
    float viewX, viewY;
    float fieldOfViewY;
    float nearZ, farZ;

    glm::mat4 projectionMatrix() const noexcept override {
        return glm::perspectiveFov(glm::radians(fieldOfViewY), viewX, viewY, nearZ, farZ);
    }
};

#endif // CAMERA_HPP