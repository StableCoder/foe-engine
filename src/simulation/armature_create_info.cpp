// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "armature_create_info.hpp"

void foeDestroyArmatureCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo) {
    auto *pCI = (foeArmatureCreateInfo *)pCreateInfo;
    pCI->~foeArmatureCreateInfo();
}
