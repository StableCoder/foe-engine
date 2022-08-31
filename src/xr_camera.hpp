// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef XR_CAMERA_HPP
#define XR_CAMERA_HPP

#include <foe/position/component/3d.hpp>
#include <foe/xr/openxr/camera_math.hpp>
#include <vulkan/vulkan.h>

struct foeXrCamera {
    // Projection Data
    XrFovf fov;
    float nearZ, farZ;

    // View Data
    foePosition3d *pPosition3D;
    XrPosef pose;

    // Graphics Data
    VkDescriptorSet descriptor{VK_NULL_HANDLE};
};

inline glm::mat4 foeXrCameraProjectionMatrix(foeXrCamera const *pXrCamera) {
    return foeOpenXrProjectionMatrix(pXrCamera->fov, pXrCamera->nearZ, pXrCamera->farZ);
}

inline glm::mat4 foeXrCameraViewMatrix(foeXrCamera const *pXrCamera) {
    glm::mat4 rot = glm::mat4_cast(foeOpenXrPoseOrientation(pXrCamera->pose));
    glm::vec3 pos = pXrCamera->pPosition3D->position + foeOpenXrPosePosition(pXrCamera->pose);

    glm::mat4 view = glm::translate(glm::mat4(1.f), pos) * rot;
    view = glm::inverse(view);

    return view;
}

#endif // XR_CAMERA_HPP