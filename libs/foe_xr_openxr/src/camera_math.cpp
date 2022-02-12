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