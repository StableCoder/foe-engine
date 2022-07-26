// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/vertex_descriptor.hpp>

void cleanup_foeVertexDescriptor(foeVertexDescriptor *pData) {
    cleanup_foeGfxVertexDescriptor(&pData->vertexDescriptor);
}