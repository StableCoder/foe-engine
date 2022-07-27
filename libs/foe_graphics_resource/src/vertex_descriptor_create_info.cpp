// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/vertex_descriptor_create_info.hpp>

#include <stdlib.h>

void foeDestroyVertexDescriptorCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo) {
    auto *pCI = (foeVertexDescriptorCreateInfo *)pCreateInfo;

    if (pCI->pInputAttributes)
        free(pCI->pInputAttributes);
    if (pCI->pInputBindings)
        free(pCI->pInputBindings);
}