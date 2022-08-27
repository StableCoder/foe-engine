// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/physics/compare.h>

#include <foe/physics/component/rigid_body.h>
#include <foe/physics/resource/collision_shape_create_info.hpp>

extern "C" bool compare_foeRigidBody(foeRigidBody const *pData1, foeRigidBody const *pData2) {
    // float - mass
    if (pData1->mass != pData2->mass) {
        return false;
    }

    // foeResourceID - collisionShape
    if (pData1->collisionShape != pData2->collisionShape) {
        return false;
    }

    return true;
}

extern "C" bool compare_foeCollisionShapeCreateInfo(foeCollisionShapeCreateInfo const *pData1,
                                                    foeCollisionShapeCreateInfo const *pData2) {
    // glm::vec3 - boxSize
    if (pData1->boxSize != pData2->boxSize) {
        return false;
    }

    return true;
}
