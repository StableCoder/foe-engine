// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/image_create_info.hpp>

void foeDestroyImageCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo) {
    auto *pCI = (foeImageCreateInfo *)pCreateInfo;
    pCI->~foeImageCreateInfo();
}