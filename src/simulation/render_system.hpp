// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RENDER_SYSTEM_HPP
#define RENDER_SYSTEM_HPP

#include <foe/graphics/session.h>
#include <foe/handle.h>
#include <foe/position/component/3d_pool.h>
#include <foe/resource/pool.h>
#include <foe/result.h>

#include "animated_bone_state_pool.h"
#include "render_state_pool.h"

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeRenderSystem)

foeResultSet foeCreateRenderSystem(foeRenderSystem *pRenderSystem);

void foeDestroyRenderSystem(foeRenderSystem renderSystem);

foeResultSet foeInitializeRenderSystemGraphics(foeRenderSystem renderSystem,
                                               foeGfxSession gfxSession,
                                               foeResourcePool resourcePool,
                                               foeRenderStatePool renderStatePool,
                                               foePosition3dPool positionPool,
                                               foeAnimatedBoneStatePool animatedBoneStatePool);

void foeDeinitializeRenderSystemGraphics(foeRenderSystem renderSystem);

foeResultSet foeProcessRenderSystem(foeRenderSystem renderSystem);

foeResultSet foeProcessRenderSystemGraphics(foeRenderSystem renderSystem, uint32_t frameIndex);

#ifdef __cplusplus
}
#endif

// TEMP

struct RenderDataSet {
    foeEntityID entity;

    foeResource vertexDescriptor;
    foeResource bonedVertexDescriptor;
    foeResource material;
    foeResource mesh;

    uint32_t armatureIndex;
};

#include <vector>
#include <vulkan/vulkan.h>

std::vector<RenderDataSet> const &getRenderDataSets(foeRenderSystem renderSystem);
VkDescriptorSet const *getPositionDescriptorSets(foeRenderSystem renderSystem, uint32_t frameIndex);
VkDescriptorSet const *getArmatureDescriptorSets(foeRenderSystem renderSystem, uint32_t frameIndex);

#endif // RENDER_SYSTEM_HPP