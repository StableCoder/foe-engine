// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef VK_ANIMATION_HPP
#define VK_ANIMATION_HPP

#include <foe/error_code.h>
#include <foe/graphics/session.h>
#include <foe/graphics/type_defs.h>
#include <foe/resource/pool.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <array>

class foeArmatureStatePool;
class foeRenderStatePool;

class VkAnimationPool {
  public:
    foeResultSet initialize(foeResourcePool resourcePool,
                            foeArmatureStatePool *pArmatureStatePool,
                            foeRenderStatePool *pRenderStatePool);
    void deinitialize();
    bool initialized() const noexcept;

    foeResultSet initializeGraphics(foeGfxSession gfxSession);
    void deinitializeGraphics();
    bool initializedGraphics() const noexcept;

    VkResult uploadBoneOffsets(uint32_t frameIndex);

  public:
    struct UniformBuffer {
        VkBuffer buffer;
        VmaAllocation alloc;
        uint32_t capacity;
    };

    // Resources
    foeResourcePool mResourcePool{FOE_NULL_HANDLE};

    // Components
    foeArmatureStatePool *mpArmatureStatePool{nullptr};
    foeRenderStatePool *mpRenderStatePool{nullptr};

    // Graphics
    VkDevice mDevice{VK_NULL_HANDLE};
    VmaAllocator mAllocator{VK_NULL_HANDLE};

    VkDescriptorSetLayout mBoneSetLayout{VK_NULL_HANDLE};
    uint32_t mBoneSetBinding;

    uint32_t maxBones{64};

    uint32_t mMinUniformBufferOffsetAlignment{0};

    std::array<UniformBuffer, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> mModelBuffers{};
    std::array<VkDescriptorPool, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> mModelDescriptorPools{};

    std::array<UniformBuffer, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> mBoneBuffers{};
    std::array<VkDescriptorPool, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> mBoneDescriptorPools{};
};

#endif // VK_ANIMATION_HPP