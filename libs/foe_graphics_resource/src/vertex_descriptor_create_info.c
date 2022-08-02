// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/vertex_descriptor_create_info.h>

#include <stdlib.h>

void foeCleanup_foeVertexDescriptorCreateInfo(foeVertexDescriptorCreateInfo *pCreateInfo) {
    if (pCreateInfo->pInputAttributes) {
        free(pCreateInfo->pInputAttributes);
    }

    if (pCreateInfo->pInputBindings) {
        free(pCreateInfo->pInputBindings);
    }
}