// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BT_GLM_CONVERSION_HPP
#define BT_GLM_CONVERSION_HPP

#include <btBulletDynamicsCommon.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

glm::vec3 btToGlmVec3(btVector3 const &vec) noexcept;
btVector3 glmToBtVec3(glm::vec3 const &vec) noexcept;

glm::quat btToGlmQuat(btQuaternion const &quat) noexcept;
btQuaternion glmToBtQuat(glm::quat const &quat) noexcept;

glm::mat4 btToGlmMat4(btTransform const &transform) noexcept;
btTransform glmToBtTransform(glm::vec3 const &position, glm::quat const &orientation) noexcept;
btTransform glmToBtTransform(glm::mat4 const &matrix);

// Inline Implementations

inline glm::vec3 btToGlmVec3(btVector3 const &vec) noexcept {
    return glm::vec3{vec.getX(), vec.getY(), vec.getZ()};
}

inline btVector3 glmToBtVec3(glm::vec3 const &vec) noexcept {
    return btVector3{vec.x, vec.y, vec.z};
}

inline glm::quat btToGlmQuat(btQuaternion const &quat) noexcept {
    return glm::quat{quat.getW(), quat.getX(), quat.getY(), quat.getZ()};
}

inline btQuaternion glmToBtQuat(glm::quat const &quat) noexcept {
    return btQuaternion{quat.x, quat.y, quat.z, quat.w};
}

inline glm::mat4 btToGlmMat4(btTransform const &transform) noexcept {
    glm::mat4 matrix;
    transform.getOpenGLMatrix(reinterpret_cast<btScalar *>(&matrix));
    return matrix;
}

inline btTransform glmToBtTransform(glm::vec3 const &position,
                                    glm::quat const &orientation) noexcept {
    return btTransform{glmToBtQuat(orientation), glmToBtVec3(position)};
}

inline btTransform glmToBtTransform(glm::mat4 const &matrix) {
    btTransform transform;
    transform.setFromOpenGLMatrix(reinterpret_cast<btScalar const *>(&matrix));
    return transform;
}

#endif // BT_GLM_CONVERSION_HPP