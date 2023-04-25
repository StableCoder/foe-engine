// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_XR_OPENXR_CAMERA_HPP
#define FOE_XR_OPENXR_CAMERA_HPP

#include <foe/xr/export.h>
#include <glm/glm.hpp>
#include <openxr/openxr.h>

FOE_XR_EXPORT
glm::mat4 foeOpenXrProjectionMatrix(XrFovf const &fieldOfView, float nearZ, float farZ);

FOE_XR_EXPORT
glm::quat foeOpenXrPoseOrientation(XrPosef const &pose);

FOE_XR_EXPORT
glm::vec3 foeOpenXrPosePosition(XrPosef const &pose);

#endif // FOE_XR_OPENXR_CAMERA_HPP