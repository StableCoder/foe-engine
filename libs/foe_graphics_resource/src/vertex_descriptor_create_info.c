// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/vertex_descriptor_create_info.h>

#include <stdlib.h>

void foeDestroyVertexDescriptorCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo) {
    foeVertexDescriptorCreateInfo *pCI = (foeVertexDescriptorCreateInfo *)pCreateInfo;

    if (pCI->pInputAttributes)
        free(pCI->pInputAttributes);
    if (pCI->pInputBindings)
        free(pCI->pInputBindings);
}