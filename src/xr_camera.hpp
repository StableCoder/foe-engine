// Copyright (C) 2020-2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef XR_CAMERA_HPP
#define XR_CAMERA_HPP

#include <foe/xr/openxr/camera_math.hpp>

#include "simulation/camera.hpp"

struct foeXrCamera : public foeCameraBase {
    // Projection Data
    XrFovf fov;
    float nearZ, farZ;

    glm::mat4 projectionMatrix() const noexcept override {
        return foeOpenXrProjectionMatrix(fov, nearZ, farZ);
    }

    // View Data
    foePosition3d *pPosition3D;
    XrPosef pose;

    glm::mat4 viewMatrix() const noexcept {
        glm::mat4 rot = glm::mat4_cast(foeOpenXrPoseOrientation(pose));
        glm::vec3 pos = pPosition3D->position + foeOpenXrPosePosition(pose);

        glm::mat4 view = glm::translate(glm::mat4(1.f), pos) * rot;
        view = glm::inverse(view);

        return view;
    }
};

#endif // XR_CAMERA_HPP