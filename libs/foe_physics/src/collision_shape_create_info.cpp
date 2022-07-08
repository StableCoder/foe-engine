// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/physics/resource/collision_shape_create_info.hpp>

void foeDestroyCollisionShapeCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo) {
    auto *pCI = (foeCollisionShapeCreateInfo *)pCreateInfo;
    pCI->~foeCollisionShapeCreateInfo();
}
