// Copyright (C) 2022-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/vertex_descriptor.h>

extern "C" void cleanup_foeVertexDescriptor(foeVertexDescriptor *pData) {
    cleanup_foeGfxVkVertexDescriptor(&pData->vertexDescriptor);
}