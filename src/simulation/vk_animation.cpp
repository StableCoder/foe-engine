// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "vk_animation.hpp"

#include <foe/graphics/resource/mesh.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/vk/session.h>
#include <glm/glm.hpp>

#include "../result.h"
#include "../vk_result.h"
#include "armature.hpp"
#include "armature_state_pool.hpp"
#include "type_defs.h"

foeResultSet VkAnimationPool::initialize(foeResourcePool resourcePool,
                                         foeArmatureStatePool *pArmatureStatePool,
                                         foeRenderStatePool renderStatePool) {
    if (resourcePool == FOE_NULL_HANDLE || pArmatureStatePool == nullptr ||
        renderStatePool == FOE_NULL_HANDLE)
        return to_foeResult(FOE_BRINGUP_INITIALIZATION_FAILED);

    // External
    mResourcePool = resourcePool;

    mpArmatureStatePool = pArmatureStatePool;
    mRenderStatePool = renderStatePool;

    return to_foeResult(FOE_BRINGUP_SUCCESS);
}

void VkAnimationPool::deinitialize() {
    // External
    mRenderStatePool = FOE_NULL_HANDLE;
    mpArmatureStatePool = nullptr;

    mResourcePool = FOE_NULL_HANDLE;
}

bool VkAnimationPool::initialized() const noexcept { return mResourcePool != FOE_NULL_HANDLE; }

foeResultSet VkAnimationPool::initializeGraphics(foeGfxSession gfxSession) {
    if (!initialized())
        return to_foeResult(FOE_BRINGUP_NOT_INITIALIZED);

    // External
    mDevice = foeGfxVkGetDevice(gfxSession);
    mAllocator = foeGfxVkGetAllocator(gfxSession);

    // Internal
    VkResult vkResult = VK_SUCCESS;

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
        vkResult = vkCreateDescriptorPool(mDevice, &poolCI, nullptr, &it);
        if (vkResult != VK_SUCCESS) {
            goto INITIALIZATION_FAILED;
        }
    }

    for (auto &it : mBoneDescriptorPools) {
        vkResult = vkCreateDescriptorPool(mDevice, &poolCI, nullptr, &it);
        if (vkResult != VK_SUCCESS) {
            goto INITIALIZATION_FAILED;
        }
    }

INITIALIZATION_FAILED:
    if (vkResult != VK_SUCCESS) {
        deinitialize();
    }

    return vk_to_foeResult(vkResult);
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
    VkResult vkResult{VK_SUCCESS};

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

        vkResult = vmaCreateBuffer(mAllocator, &bufferCI, &allocCI, &modelUniform.buffer,
                                   &modelUniform.alloc, nullptr);
        if (vkResult != VK_SUCCESS) {
            return vkResult;
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

        vkResult = vmaCreateBuffer(mAllocator, &bufferCI, &allocCI, &boneUniform.buffer,
                                   &boneUniform.alloc, nullptr);
        if (vkResult != VK_SUCCESS) {
            return vkResult;
        }

        boneUniform.capacity = 1024;
    }

    vkResetDescriptorPool(mDevice, mBoneDescriptorPools[frameIndex], 0);

    glm::mat4 *pBufferData;
    VkDeviceSize offset = 0;
    vmaMapMemory(mAllocator, boneUniform.alloc, reinterpret_cast<void **>(&pBufferData));

    foeEntityID const *pID = foeEcsComponentPoolIdPtr(mRenderStatePool);
    foeEntityID const *const pEndID = pID + foeEcsComponentPoolSize(mRenderStatePool);
    foeRenderState *pRenderState = (foeRenderState *)foeEcsComponentPoolDataPtr(mRenderStatePool);

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

        foeResource mesh{FOE_NULL_HANDLE};
        foeResource armature{FOE_NULL_HANDLE};

        do {
            mesh = foeResourcePoolFind(mResourcePool, pRenderState->mesh);

            if (mesh == FOE_NULL_HANDLE) {
                mesh =
                    foeResourcePoolAdd(mResourcePool, pRenderState->mesh,
                                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH, sizeof(foeMesh));
            }
        } while (mesh == FOE_NULL_HANDLE);

        do {
            armature = foeResourcePoolFind(mResourcePool, pArmatureState->armatureID);

            if (armature == FOE_NULL_HANDLE) {
                armature =
                    foeResourcePoolAdd(mResourcePool, pArmatureState->armatureID,
                                       FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE, sizeof(foeArmature));
            }
        } while (armature == FOE_NULL_HANDLE);

        if (foeResourceGetState(mesh) != FOE_RESOURCE_LOAD_STATE_LOADED ||
            foeResourceGetState(armature) != FOE_RESOURCE_LOAD_STATE_LOADED ||
            pArmatureState->armatureState.empty()) {
            continue;
        }

        // Get Resource Pointers
        foeMesh const *pMesh = (foeMesh const *)foeResourceGetData(mesh);
        foeArmature const *pArmature = (foeArmature const *)foeResourceGetData(armature);

        glm::mat4 lastBone = glm::mat4{1.f};
        for (auto const &bone : pMesh->gfxBones) {
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

            vkResult = vkAllocateDescriptorSets(mDevice, &setAI, &pRenderState->boneDescriptorSet);
            if (vkResult != VK_SUCCESS) {
                return vkResult;
            }

            VkDescriptorBufferInfo bufferInfo{
                .buffer = boneUniform.buffer,
                .offset = offset,
                .range = sizeof(glm::mat4) * pMesh->gfxBones.size(),
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
        offset += sizeof(glm::mat4) * pMesh->gfxBones.size();
    }

    vmaUnmapMemory(mAllocator, boneUniform.alloc);

    return vkResult;
}