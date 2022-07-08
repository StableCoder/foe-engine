// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/xr/openxr/camera_math.hpp>

#include <glm/ext.hpp>

glm::mat4 foeOpenXrProjectionMatrix(XrFovf const &fieldOfView, float nearZ, float farZ) {
    float left = tan(fieldOfView.angleLeft);
    float right = tan(fieldOfView.angleRight);
    float up = tan(fieldOfView.angleUp);
    float down = tan(fieldOfView.angleDown);

    float width = right - left;
    float height = up - down;

    return glm::mat4{
        // Col 1
        2.f / width,
        0,
        0,
        0,
        // Col 2
        0,
        2.f / height,
        0,
        0,
        // Col 3
        (right + left) / width,
        (up + down) / height,
        -farZ / (farZ - nearZ),
        -1,
        // Col 4
        0,
        0,
        -(farZ * nearZ) / (farZ - nearZ),
        0,
    };
}

glm::quat foeOpenXrPoseOrientation(XrPosef const &pose) {
    return glm::quat{pose.orientation.w * -1.f, pose.orientation.x, pose.orientation.y * -1.f,
                     pose.orientation.z};
}

glm::vec3 foeOpenXrPosePosition(XrPosef const &pose) {
    return glm::vec3{pose.position.x, -pose.position.y, pose.position.z};
}