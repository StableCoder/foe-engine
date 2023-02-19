// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RENDER_SYSTEM_ARMATURE_HPP
#define RENDER_SYSTEM_ARMATURE_HPP

#include <foe/graphics/session.h>
#include <foe/graphics/type_defs.h>
#include <foe/resource/resource.h>
#include <foe/result.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "armature_state.hpp"

#include <vector>

struct RenderSystemArmatureGpuData {
    size_t bufferSize;
    VkBuffer buffer;
    VmaAllocation alloc;
    uint32_t descriptorCount;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet *pDescriptorSets;
};

struct RenderSystemArmatureData {
    struct ArmatureBoneAlloc {
        size_t offset;
        size_t size;
    };

    size_t dataAlignment;

    // CPU
    size_t armatureCapacity;
    ArmatureBoneAlloc *pArmatureAllocations;

    size_t boneDataSize;
    void *pBoneData;

    std::vector<ArmatureBoneAlloc> freeAllocations;

    // GPU
    VkDescriptorSetLayout armatureLayout;
    uint32_t armatureBinding;

    RenderSystemArmatureGpuData armatureGpuData[FOE_GRAPHICS_MAX_BUFFERED_FRAMES];
};

[[nodiscard]] foeResultSet initializeArmatureData(foeGfxSession gfxSession,
                                                  RenderSystemArmatureData &armatureData);

void deinitializeArmatureData(foeGfxSession gfxSession, RenderSystemArmatureData &armatureData);

[[nodiscard]] foeResultSet getArmatureData(RenderSystemArmatureData &armatureData,
                                           foeAnimatedBoneState const *pAnimatedBoneState,
                                           foeResource mesh,
                                           uint32_t &armatureIndex);

void clearArmatureData(RenderSystemArmatureData &armatureData, uint32_t &armatureIndex);

[[nodiscard]] foeResultSet prepareArmatureGpuData(RenderSystemArmatureData &armatureData,
                                                  foeGfxSession gfxSession,
                                                  uint32_t frameIndex);

#endif // RENDER_SYSTEM_ARMATURE_HPP