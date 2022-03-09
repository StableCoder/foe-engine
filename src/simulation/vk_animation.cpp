/*
    Copyright (C) 2021-2022 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "vk_animation.hpp"

#include <foe/graphics/resource/mesh.hpp>
#include <foe/graphics/resource/mesh_pool.hpp>
#include <foe/graphics/vk/session.hpp>
#include <glm/glm.hpp>
#include <vk_error_code.hpp>
#include <vulkan/vulkan_core.h>

#include "../error_code.hpp"
#include "armature.hpp"
#include "armature_pool.hpp"
#include "armature_state_pool.hpp"
#include "render_state_pool.hpp"
#include "type_defs.h"

auto VkAnimationPool::initialize(foeArmaturePool *pArmaturePool,
                                 foeMeshPool *pMeshPool,
                                 foeArmatureStatePool *pArmatureStatePool,
                                 foeRenderStatePool *pRenderStatePool) -> std::error_code {
    if (pArmaturePool == nullptr || pMeshPool == nullptr || pArmatureStatePool == nullptr ||
        pRenderStatePool == nullptr)
        return FOE_BRINGUP_INITIALIZATION_FAILED;

    // External
    mpArmaturePool = pArmaturePool;
    mpMeshPool = pMeshPool;

    mpArmatureStatePool = pArmatureStatePool;
    mpRenderStatePool = pRenderStatePool;

    return VK_SUCCESS;
}

void VkAnimationPool::deinitialize() {
    // External
    mpRenderStatePool = nullptr;
    mpArmatureStatePool = nullptr;

    mpMeshPool = nullptr;
    mpArmaturePool = nullptr;
}

bool VkAnimationPool::initialized() const noexcept { return mpArmaturePool != nullptr; }

auto VkAnimationPool::initializeGraphics(foeGfxSession gfxSession) -> std::error_code {
    if (!initialized())
        return FOE_BRINGUP_NOT_INITIALIZED;

    // External
    mDevice = foeGfxVkGetDevice(gfxSession);
    mAllocator = foeGfxVkGetAllocator(gfxSession);

    // Internal
    VkResult res;

    mBoneSetLayout = foeGfxVkGetBuiltinLayout(
        gfxSession, foeBuiltinDescriptorSetLayoutFlagBits::
                        FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_BONE_STATE_MATRICES);

    mBoneSetBinding = foeGfxVkGetBuiltinSetLayoutIndex(
        gfxSession, foeBuiltinDescriptorSetLayoutFlagBits::
                        FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_BONE_STATE_MATRICES);

    auto physicalDevice = foeGfxVkGetPhysicalDevice(gfxSession);
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    mMinUniformBufferOffsetAlignment = std::max(static_cast<VkDeviceSize>(sizeof(glm::mat4)),
                                                properties.limits.minUniformBufferOffsetAlignment);

    VkDescriptorPoolSize size{
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1024,
    };

    VkDescriptorPoolCreateInfo poolCI{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1024,
        .poolSizeCount = 1,
        .pPoolSizes = &size,
    };

    for (auto &it : mModelDescriptorPools) {
        res = vkCreateDescriptorPool(mDevice, &poolCI, nullptr, &it);
        if (res != VK_SUCCESS) {
            goto INITIALIZATION_FAILED;
        }
    }

    for (auto &it : mBoneDescriptorPools) {
        res = vkCreateDescriptorPool(mDevice, &poolCI, nullptr, &it);
        if (res != VK_SUCCESS) {
            goto INITIALIZATION_FAILED;
        }
    }

INITIALIZATION_FAILED:
    if (res != VK_SUCCESS) {
        deinitialize();
    }

    return res;
}

void VkAnimationPool::deinitializeGraphics() {
    // Internal
    for (auto &it : mBoneDescriptorPools) {
        if (it != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(mDevice, it, nullptr);
        }
        it = VK_NULL_HANDLE;
    }

    for (auto &it : mModelDescriptorPools) {
        if (it != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(mDevice, it, nullptr);
        }
        it = VK_NULL_HANDLE;
    }

    for (auto &it : mBoneBuffers) {
        if (it.buffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(mAllocator, it.buffer, it.alloc);
        }
        it = {};
    }

    for (auto &it : mModelBuffers) {
        if (it.buffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(mAllocator, it.buffer, it.alloc);
        }
        it = {};
    }

    // External
    mAllocator = VK_NULL_HANDLE;
    mDevice = VK_NULL_HANDLE;
}

bool VkAnimationPool::initializedGraphics() const noexcept { return mDevice != VK_NULL_HANDLE; }

VkResult VkAnimationPool::uploadBoneOffsets(uint32_t frameIndex) {
    VkResult res{VK_SUCCESS};

    UniformBuffer &modelUniform = mModelBuffers[frameIndex];
    UniformBuffer &boneUniform = mBoneBuffers[frameIndex];

    if (modelUniform.capacity == 0) {
        VkBufferCreateInfo bufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = sizeof(glm::mat4),
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        };

        VmaAllocationCreateInfo allocCI{
            .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
        };

        res = vmaCreateBuffer(mAllocator, &bufferCI, &allocCI, &modelUniform.buffer,
                              &modelUniform.alloc, nullptr);
        if (res != VK_SUCCESS) {
            return res;
        }

        modelUniform.capacity = 1;
    }

    if (boneUniform.capacity < 1024) {
        VkBufferCreateInfo bufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = sizeof(glm::mat4) * 1024,
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        };

        VmaAllocationCreateInfo allocCI{
            .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
        };

        res = vmaCreateBuffer(mAllocator, &bufferCI, &allocCI, &boneUniform.buffer,
                              &boneUniform.alloc, nullptr);
        if (res != VK_SUCCESS) {
            return res;
        }

        boneUniform.capacity = 1024;
    }

    vkResetDescriptorPool(mDevice, mBoneDescriptorPools[frameIndex], 0);

    glm::mat4 *pBufferData;
    VkDeviceSize offset = 0;
    vmaMapMemory(mAllocator, boneUniform.alloc, reinterpret_cast<void **>(&pBufferData));

    auto *pID = mpRenderStatePool->begin();
    auto const *pEndID = mpRenderStatePool->end();
    auto *pRenderState = mpRenderStatePool->begin<1>();

    for (; pID != pEndID; ++pID, ++pRenderState) {
        // Make sure the buffer offset matches the minimum allowed alignment
        {
            auto alignment = offset / mMinUniformBufferOffsetAlignment;
            if (offset != alignment * mMinUniformBufferOffsetAlignment) {
                // Not maching an alignment, need to move offset up a little
                offset = (alignment + 1) * mMinUniformBufferOffsetAlignment;
            }
        }

        // Only need bone state data if we have an associated armature.
        foeArmatureState const *pArmatureState{nullptr};

        auto searchOffset = mpArmatureStatePool->find(*pID);
        if (searchOffset != mpArmatureStatePool->size()) {
            pArmatureState = mpArmatureStatePool->begin<1>() + searchOffset;
        } else {
            continue;
        }

        foeMesh *pMesh = mpMeshPool->find(pRenderState->mesh);
        foeResource armature = mpArmaturePool->find(pArmatureState->armatureID);

        if (pMesh == nullptr || armature == FOE_NULL_HANDLE ||
            pMesh->getState() != foeResourceState::Loaded ||
            foeResourceGetState(armature) != foeResourceLoadState::Loaded ||
            pArmatureState->armatureState.empty()) {
            continue;
        }
        foeArmature const *pArmature = (foeArmature const *)foeResourceGetData(armature);

        glm::mat4 lastBone = glm::mat4{1.f};
        for (auto const &bone : pMesh->data.gfxBones) {
            // Find the matching armature node, if it exists
            foeArmatureNode const *pArmatureNode{nullptr};
            size_t armatureNodeIndex;
            for (size_t i = 0; i < pArmature->armature.size(); ++i) {
                if (pArmature->armature[i].name == bone.name) {
                    pArmatureNode = &pArmature->armature[i];
                    armatureNodeIndex = i;
                    break;
                }
            }

            if (pArmatureNode == nullptr) {
                // Didn't find a matching node,
                lastBone = glm::mat4{1.f};
            } else {
                lastBone = pArmatureState->armatureState[armatureNodeIndex];
            }

            *pBufferData = lastBone * bone.offsetMatrix;
            ++pBufferData;
        }

        { // Set bone descriptor set
            VkDescriptorSetAllocateInfo setAI{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = mBoneDescriptorPools[frameIndex],
                .descriptorSetCount = 1U,
                .pSetLayouts = &mBoneSetLayout,
            };

            res = vkAllocateDescriptorSets(mDevice, &setAI, &pRenderState->boneDescriptorSet);
            if (res != VK_SUCCESS) {
                return res;
            }

            VkDescriptorBufferInfo bufferInfo{
                .buffer = boneUniform.buffer,
                .offset = offset,
                .range = sizeof(glm::mat4) * pMesh->data.gfxBones.size(),
            };

            VkWriteDescriptorSet writeSet{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = pRenderState->boneDescriptorSet,
                .dstBinding = mBoneSetBinding,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &bufferInfo,
            };

            vkUpdateDescriptorSets(mDevice, 1, &writeSet, 0, nullptr);
        }

        // Move the offset into the buffer by however many bones we put in
        offset += sizeof(glm::mat4) * pMesh->data.gfxBones.size();
    }

    vmaUnmapMemory(mAllocator, boneUniform.alloc);

    return res;
}