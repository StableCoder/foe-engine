// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RENDER_STATE_H
#define RENDER_STATE_H

#include <foe/ecs/id.h>

#ifdef __cplusplus
extern "C" {
#endif

struct foeRenderState {
    foeId vertexDescriptor;
    foeId bonedVertexDescriptor;
    foeId material;
    foeId mesh;
};

#ifdef __cplusplus
}
#endif

#endif // RENDER_STATE_H