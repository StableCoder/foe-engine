// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

struct foeCamera {
    // Projection Data
    float viewX, viewY;
    float fieldOfViewY;
    float nearZ, farZ;

    // Graphics Data
    VkDescriptorSet descriptor{VK_NULL_HANDLE};
};

inline glm::mat4 foeCameraProjectionMatrix(foeCamera const *pCamera) {
    return glm::perspectiveFov(glm::radians(pCamera->fieldOfViewY), pCamera->viewX, pCamera->viewY,
                               pCamera->nearZ, pCamera->farZ);
}

#endif // CAMERA_HPP