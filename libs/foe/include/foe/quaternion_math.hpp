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

#ifndef FOE_QUATERNION_MATH_HPP
#define FOE_QUATERNION_MATH_HPP

#include <glm/ext.hpp>
#include <glm/glm.hpp>

/// Returns the forward normalized direction vector
inline glm::vec3 forwardVec(glm::quat const &quaternion) noexcept {
    // Can be derived from the 3x3 view matrix at positions X[0][2], Y[1][2] and Z[2][2]
    return glm::vec3{2 * (quaternion.x * quaternion.z - quaternion.w * quaternion.y),
                     2 * (quaternion.y * quaternion.z + quaternion.w * quaternion.x),
                     1 - 2 * (quaternion.x * quaternion.x + quaternion.y * quaternion.y)};
}

/// Returns the left strafe normalized direction vector
inline glm::vec3 leftVec(glm::quat const &quaternion) noexcept {
    // Can be derived from the 3x3 view matrix at positions X[0][0], Y[1][0] and Z[2][0]
    return glm::vec3{1 - 2 * (quaternion.y * quaternion.y + quaternion.z * quaternion.z),
                     2 * (quaternion.x * quaternion.y - quaternion.w * quaternion.z),
                     2 * (quaternion.x * quaternion.z + quaternion.w * quaternion.y)};
}

/// Returns the up normalized direction vector
inline glm::vec3 upVec(glm::quat const &quaternion) noexcept {
    // Can be derived from the 3x3 view matrix at positions X[0][1], Y[1][1] and Z[2][1]
    return glm::vec3{2 * (quaternion.x * quaternion.y + quaternion.w * quaternion.z),
                     1 - 2 * (quaternion.x * quaternion.x + quaternion.z * quaternion.z),
                     2 * (quaternion.y * quaternion.z - quaternion.w * quaternion.x)};
}

/** @brief Returns the orientation pitch (up/down) by the given radian value
 * @param quaternion Initial orientation value
 * @param radians Magnitude to rotate by
 */
inline glm::quat changePitch(glm::quat const &quaternion, float radians) noexcept {
    glm::quat change = glm::quat{glm::vec3{radians, 0.f, 0.f}};
    return glm::normalize(change * quaternion);
}

/** @brief Returns the orientation yaw (left/right) by the given radian value
 * @param quaternion Initial orientation value
 * @param radians Magnitude to rotate by
 */
inline glm::quat changeYaw(glm::quat const &quaternion, float radians) noexcept {
    glm::quat change = glm::quat{glm::vec3{0.f, radians, 0.f}};
    return glm::normalize(change * quaternion);
}

/** @brief Returns the orientation roll (right/left) by the given radian value
 * @param quaternion Initial orientation value
 * @param radians Magnitude to rotate by
 */
inline glm::quat changeRoll(glm::quat const &quaternion, float radians) noexcept {
    glm::quat change = glm::quat{glm::vec3{0.f, 0.f, radians}};
    return glm::normalize(change * quaternion);
}

#endif // FOE_QUATERNION_MATH_HPP