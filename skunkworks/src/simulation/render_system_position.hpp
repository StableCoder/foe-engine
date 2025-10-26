// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RENDER_POSITION_SUBSYSTEM_HPP
#define RENDER_POSITION_SUBSYSTEM_HPP

#include <foe/external/vk_mem_alloc.h>
#include <foe/graphics/session.h>
#include <foe/graphics/type_defs.h>
#include <foe/graphics/vk/session.h>
#include <foe/position/component/3d.hpp>
#include <foe/result.h>
#include <vulkan/vulkan.h>

struct RenderSystemPositionGpuData {
    size_t itemCapacity; // Capacity in number of items
    VkBuffer buffer;
    VmaAllocation alloc;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet *pDescriptorSets;
};

struct RenderSystemPositionData {
    // CPU Data
    void *pCpuBuffer;
    size_t itemCount;
    size_t itemCapacity;

    // GPU Data
    VkDescriptorSetLayout positionLayout;
    uint32_t positionBinding;

    RenderSystemPositionGpuData gpuData[FOE_GRAPHICS_MAX_BUFFERED_FRAMES];

    // Other
    size_t alignment;
};

[[nodiscard]] foeResultSet initializePositionData(foeGfxSession gfxSession,
                                                  RenderSystemPositionData &positionData);

void deinitializePositionData(foeGfxSession gfxSession, RenderSystemPositionData &positionData);

[[nodiscard]] foeResultSet insertPositionData(RenderSystemPositionData &positionData,
                                              size_t index,
                                              foePosition3d const *pPositionData);

void removePositionData(RenderSystemPositionData &positionData, size_t index);

[[nodiscard]] foeResultSet preparePositionGpuData(RenderSystemPositionData &positionData,
                                                  foeGfxSession gfxSession,
                                                  uint32_t frameIndex);

#endif // RENDER_POSITION_SUBSYSTEM_HPP