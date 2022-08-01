// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_CREATE_INFO_HPP
#define FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_CREATE_INFO_HPP

#include <foe/physics/export.h>
#include <foe/resource/create_info.h>
#include <glm/glm.hpp>

struct foeCollisionShapeCreateInfo {
    glm::vec3 boxSize;
};

#endif // FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_CREATE_INFO_HPP