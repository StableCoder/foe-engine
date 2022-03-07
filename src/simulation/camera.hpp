/*
    Copyright (C) 2020-2021 George Cave.

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