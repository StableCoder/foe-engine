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

#ifndef FOE_XR_OPENXR_CAMERA_HPP
#define FOE_XR_OPENXR_CAMERA_HPP

#include <foe/xr/export.h>
#include <glm/glm.hpp>
#include <openxr/openxr.h>

FOE_XR_EXPORT glm::mat4 foeOpenXrProjectionMatrix(XrFovf const &fieldOfView,
                                                  float nearZ,
                                                  float farZ);

FOE_XR_EXPORT glm::quat foeOpenXrPoseOrientation(XrPosef const &pose);

FOE_XR_EXPORT glm::vec3 foeOpenXrPosePosition(XrPosef const &pose);

#endif // FOE_XR_OPENXR_CAMERA_HPP