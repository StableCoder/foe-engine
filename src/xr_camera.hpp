/*
    Copyright (C) 2020 George Cave.

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

#ifndef XR_CAMERA_HPP
#define XR_CAMERA_HPP

#include <foe/xr/camera_math.hpp>

#include "camera.hpp"

struct foeXrCamera : public foeCameraBase {
    // Projection Data
    XrFovf fov;
    float nearZ, farZ;

    glm::mat4 projectionMatrix() const noexcept override {
        return foeXrProjectionMatrix(fov, nearZ, farZ);
    }

    // View Data
    glm::vec3 startPos;
    XrPosef pose;

    glm::mat4 viewMatrix() const noexcept override {
        glm::mat4 rot = glm::mat4_cast(foeXrPoseOrientation(pose));
        glm::vec3 pos = startPos + foeXrPosePosition(pose);

        glm::mat4 view = glm::translate(glm::mat4(1.f), pos) * rot;
        view = glm::inverse(view);

        return view;
    }
};

#endif // XR_CAMERA_HPP