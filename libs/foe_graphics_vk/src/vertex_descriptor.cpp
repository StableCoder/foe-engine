// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/vertex_descriptor.hpp>

#include <stdlib.h>

void cleanup_foeGfxVertexDescriptor(foeGfxVertexDescriptor *pData) {
    if (pData->pVertexInputAttributes)
        free(pData->pVertexInputAttributes);
    if (pData->pVertexInputBindings)
        free(pData->pVertexInputBindings);
}