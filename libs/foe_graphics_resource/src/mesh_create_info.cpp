// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/mesh_create_info.hpp>

void foeDestroyMeshCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo) {
    auto *pCI = (foeMeshCreateInfo *)pCreateInfo;
    pCI->~foeMeshCreateInfo();
}