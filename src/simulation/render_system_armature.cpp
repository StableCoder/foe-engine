// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "render_system_armature.hpp"

#include <foe/graphics/resource/mesh.hpp>
#include <foe/graphics/vk/session.h>
#include <foe/memory_alignment.h>

#include "../result.h"
#include "../vk_result.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>

namespace {

void destroyRenderSystemArmatureGpuData(foeGfxSession gfxSession,
                                        RenderSystemArmatureGpuData const *pData) {
    free(pData->pDescriptorSets);

    if (pData->descriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(foeGfxVkGetDevice(gfxSession), pData->descriptorPool, nullptr);

    if (pData->buffer != VK_NULL_HANDLE)
        vmaDestroyBuffer(foeGfxVkGetAllocator(gfxSession), pData->buffer, pData->alloc);
}

foeResultSet createRenderSystemArmatureGpuData(foeGfxSession gfxSession,
                                               size_t bufferSize,
                                               uint32_t descriptorCount,
                                               VkDescriptorSetLayout layout,
                                               RenderSystemArmatureGpuData *pData) {
    RenderSystemArmatureGpuData newData = {
        .bufferSize = bufferSize,
        .descriptorCount = descriptorCount,
    };
    foeResultSet result = to_foeResult(FOE_BRINGUP_SUCCESS);
    VkResult vkResult = VK_SUCCESS;

    { // GPU Buffer
        VkBufferCreateInfo bufferCI = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = bufferSize,
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        };

        VmaAllocationCreateInfo allocCI = {
            .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
        };

        vkResult = vmaCreateBuffer(foeGfxVkGetAllocator(gfxSession), &bufferCI, &allocCI,
                                   &newData.buffer, &newData.alloc, NULL);
        if (vkResult != VK_SUCCESS)
            goto ARMATURE_GPU_CREATE_FAILED;
    }

    { // Descriptor Pool
        VkDescriptorPoolSize descriptorPoolSize = {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount =
                descriptorCount * 2, // TODO -  Figure out why ran out of pool memory count of 4
        };

        VkDescriptorPoolCreateInfo descriptorPoolCI = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets =
                descriptorCount * 2, // TODO -  Figure out why ran out of pool memory count of 4
            .poolSizeCount = 1,
            .pPoolSizes = &descriptorPoolSize,
        };
        vkResult = vkCreateDescriptorPool(foeGfxVkGetDevice(gfxSession), &descriptorPoolCI, NULL,
                                          &newData.descriptorPool);
        if (vkResult != VK_SUCCESS)
            goto ARMATURE_GPU_CREATE_FAILED;
    }

    { // Descriptor Sets
        newData.pDescriptorSets =
            (VkDescriptorSet *)malloc(descriptorCount * sizeof(VkDescriptorSet));
        if (newData.pDescriptorSets == nullptr) {
            result = to_foeResult(FOE_BRINGUP_ERROR_OUT_OF_MEMORY);
            goto ARMATURE_GPU_CREATE_FAILED;
        }

        VkDescriptorSetAllocateInfo descriptorSetAI = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = newData.descriptorPool,
            .descriptorSetCount = 1U,
            .pSetLayouts = &layout,
        };

        for (uint32_t i = 0; i < descriptorCount; ++i) {
            vkResult = vkAllocateDescriptorSets(foeGfxVkGetDevice(gfxSession), &descriptorSetAI,
                                                &newData.pDescriptorSets[i]);
            if (vkResult != VK_SUCCESS)
                goto ARMATURE_GPU_CREATE_FAILED;
        }
    }

ARMATURE_GPU_CREATE_FAILED:
    if (vkResult != VK_SUCCESS)
        result = vk_to_foeResult(vkResult);

    if (result.value == FOE_SUCCESS) {
        *pData = newData;
    } else {
        destroyRenderSystemArmatureGpuData(gfxSession, &newData);
    }

    return result;
}

void freeAllocation(std::vector<RenderSystemArmatureData::ArmatureBoneAlloc> &freeAllocs,
                    RenderSystemArmatureData::ArmatureBoneAlloc alloc) {
    // Find where to insert the newly freed memory in the array
    auto insertIt =
        std::lower_bound(freeAllocs.begin(), freeAllocs.end(), alloc.offset,
                         [](RenderSystemArmatureData::ArmatureBoneAlloc const &armatureData,
                            size_t const offset) { return armatureData.offset < offset; });

    // See if we can merge into the previous entry
    bool mergeBefore = false;
    auto previousIt = insertIt - 1;
    if (insertIt != freeAllocs.begin()) {
        // If the insert position is not at the beginning of the range, that means there must be a
        // previous entry we can try to merge into
        if (previousIt->offset + previousIt->size == alloc.offset) {
            mergeBefore = true;
        }
    }

    // See if we can merge into the next entry
    bool mergeAfter = false;
    auto nextIt = insertIt;
    if (insertIt != freeAllocs.end() && alloc.offset + alloc.size == nextIt->offset) {
        // The allocation we are freeing is contiguous with the next entry
        mergeAfter = true;
    }

    if (mergeBefore && mergeAfter) {
        // Merge into the 'before' entry and remove the 'after' entry
        previousIt->size += alloc.size + nextIt->size;
        freeAllocs.erase(nextIt);
    } else if (mergeBefore) {
        // Merge into the 'before' entry
        previousIt->size += alloc.size;

    } else if (mergeAfter) {
        // Merge into the 'next' entry
        nextIt->offset = alloc.offset;
        nextIt->size += alloc.size;

    } else {
        // Not contiguous at all, new entry
        freeAllocs.insert(insertIt, alloc);
    }
}

} // namespace

foeResultSet initializeArmatureData(foeGfxSession gfxSession,
                                    RenderSystemArmatureData &armatureData) {
    VkPhysicalDeviceProperties devProperties;
    vkGetPhysicalDeviceProperties(foeGfxVkGetPhysicalDevice(gfxSession), &devProperties);

    armatureData = {
        .dataAlignment = devProperties.limits.minUniformBufferOffsetAlignment,
        .armatureCapacity = 16,
        .pArmatureAllocations = (RenderSystemArmatureData::ArmatureBoneAlloc *)calloc(
            16, sizeof(RenderSystemArmatureData::ArmatureBoneAlloc)),
        .boneDataSize = 128,
        .pBoneData = malloc(128),
        .armatureLayout = foeGfxVkGetBuiltinLayout(
            gfxSession, foeBuiltinDescriptorSetLayoutFlagBits::
                            FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_BONE_STATE_MATRICES),
        .armatureBinding = foeGfxVkGetBuiltinSetLayoutIndex(
            gfxSession, foeBuiltinDescriptorSetLayoutFlagBits::
                            FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_BONE_STATE_MATRICES),
    };

    return to_foeResult(FOE_BRINGUP_SUCCESS);
}

void deinitializeArmatureData(foeGfxSession gfxSession, RenderSystemArmatureData &armatureData) {
    for (size_t i = 0; i < FOE_GRAPHICS_MAX_BUFFERED_FRAMES; ++i) {
        destroyRenderSystemArmatureGpuData(gfxSession, armatureData.armatureGpuData + i);
    }

    free(armatureData.pBoneData);
    free(armatureData.pArmatureAllocations);
}

foeResultSet prepareArmatureGpuData(RenderSystemArmatureData &armatureData,
                                    foeGfxSession gfxSession,
                                    uint32_t frameIndex) {
    foeResultSet result = to_foeResult(FOE_BRINGUP_SUCCESS);

    if (armatureData.armatureCapacity > 0) {
        if (armatureData.armatureGpuData[frameIndex].bufferSize < armatureData.boneDataSize ||
            armatureData.armatureGpuData[frameIndex].descriptorCount <
                armatureData.armatureCapacity) {
            destroyRenderSystemArmatureGpuData(gfxSession,
                                               &armatureData.armatureGpuData[frameIndex]);
            armatureData.armatureGpuData[frameIndex] = {};

            result = createRenderSystemArmatureGpuData(
                gfxSession, armatureData.boneDataSize, armatureData.armatureCapacity,
                armatureData.armatureLayout, &armatureData.armatureGpuData[frameIndex]);
            if (result.value != FOE_SUCCESS)
                return result;

            // @TODO - Write All Descriptor Sets
        } else {
            // @TODO - Update Descriptor Sets
        }

        VkDevice const vkDevice = foeGfxVkGetDevice(gfxSession);
        VkDescriptorBufferInfo bufferInfo{
            .buffer = armatureData.armatureGpuData[frameIndex].buffer,
        };

        VkWriteDescriptorSet writeSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstBinding = armatureData.armatureBinding,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &bufferInfo,
        };

        for (uint32_t i = 0; i < armatureData.armatureCapacity; ++i) {
            RenderSystemArmatureData::ArmatureBoneAlloc *pAllocation =
                armatureData.pArmatureAllocations + i;

            if (pAllocation->size == 0)
                continue;

            bufferInfo.offset = pAllocation->offset;
            bufferInfo.range = pAllocation->size;

            writeSet.dstSet = *(armatureData.armatureGpuData[frameIndex].pDescriptorSets + i);

            vkUpdateDescriptorSets(vkDevice, 1, &writeSet, 0, nullptr);
        }

        void *pMappedData;
        VkResult vkResult =
            vmaMapMemory(foeGfxVkGetAllocator(gfxSession),
                         armatureData.armatureGpuData[frameIndex].alloc, &pMappedData);
        if (vkResult != VK_SUCCESS)
            return vk_to_foeResult(vkResult);

        memcpy(pMappedData, armatureData.pBoneData, armatureData.boneDataSize);

        vmaUnmapMemory(foeGfxVkGetAllocator(gfxSession),
                       armatureData.armatureGpuData[frameIndex].alloc);
    }

    return result;
}

foeResultSet getArmatureData(RenderSystemArmatureData &armatureData,
                             foeAnimatedBoneState const *pAnimatedBoneState,
                             foeResource mesh,
                             uint32_t &armatureIndex) {
    foeMesh const *pMesh = (foeMesh const *)foeResourceGetData(mesh);

    if (pMesh->gfxBones.empty()) {
        clearArmatureData(armatureData, armatureIndex);
        return to_foeResult(FOE_BRINGUP_SUCCESS);
    }

    size_t requiredBoneDataSize =
        foeGetAlignedSize(armatureData.dataAlignment, pMesh->gfxBones.size() * sizeof(glm::mat4));

    if (armatureIndex != UINT32_MAX) {
        // If we already have allocated space for bones, check if we need to reallocate or can just
        // return
        RenderSystemArmatureData::ArmatureBoneAlloc *pArmatureAllocation =
            armatureData.pArmatureAllocations + armatureIndex;

        if (pArmatureAllocation->size == requiredBoneDataSize)
            // Its the size we need, just retrn
            return to_foeResult(FOE_BRINGUP_SUCCESS);

        // We need a different size, reallocate
        clearArmatureData(armatureData, armatureIndex);
    }

    // Armature
    RenderSystemArmatureData::ArmatureBoneAlloc *pArmatureEntry = nullptr;

    do {
        for (size_t i = 0; i < armatureData.armatureCapacity; ++i) {
            RenderSystemArmatureData::ArmatureBoneAlloc *pProspectiveEntry =
                armatureData.pArmatureAllocations + i;

            if (pProspectiveEntry->size == 0) {
                // Unused entries have size of 0
                pArmatureEntry = pProspectiveEntry;
                break;
            }
        }

        if (pArmatureEntry == nullptr) {
            // Not enough entries, realloc larger
            RenderSystemArmatureData::ArmatureBoneAlloc *pNewArmatureAllocations =
                (RenderSystemArmatureData::ArmatureBoneAlloc *)malloc(
                    armatureData.armatureCapacity * 2 *
                    sizeof(RenderSystemArmatureData::ArmatureBoneAlloc));
            if (pNewArmatureAllocations == nullptr)
                return to_foeResult(FOE_BRINGUP_ERROR_OUT_OF_MEMORY);

            memcpy(pNewArmatureAllocations, armatureData.pArmatureAllocations,
                   armatureData.armatureCapacity *
                       sizeof(RenderSystemArmatureData::ArmatureBoneAlloc));

            free(armatureData.pArmatureAllocations);
            armatureData.pArmatureAllocations = pNewArmatureAllocations;
            armatureData.armatureCapacity *= 2;
        }
    } while (pArmatureEntry == nullptr);

    // Bones
    auto proposedFreeAlloc = armatureData.freeAllocations.end();

    do {
        for (auto freeAllocIt = armatureData.freeAllocations.begin();
             freeAllocIt != armatureData.freeAllocations.end(); ++freeAllocIt) {
            if (freeAllocIt->size == requiredBoneDataSize) {
                proposedFreeAlloc = freeAllocIt;
                break;
            }

            if (proposedFreeAlloc == armatureData.freeAllocations.end()) {
                if (freeAllocIt->size > requiredBoneDataSize)
                    proposedFreeAlloc = freeAllocIt;
            } else if (freeAllocIt->size > requiredBoneDataSize &&
                       freeAllocIt->size < proposedFreeAlloc->size) {
                proposedFreeAlloc = freeAllocIt;
            }
        }

        if (proposedFreeAlloc == armatureData.freeAllocations.end()) {
            // Not blocks large enough, realloc larger
            void *pNewBoneData = malloc(armatureData.boneDataSize * 2);
            if (pNewBoneData == nullptr)
                return to_foeResult(FOE_BRINGUP_ERROR_OUT_OF_MEMORY);

            memcpy(pNewBoneData, armatureData.pBoneData, armatureData.boneDataSize);

            freeAllocation(armatureData.freeAllocations,
                           RenderSystemArmatureData::ArmatureBoneAlloc{
                               .offset = armatureData.boneDataSize,
                               .size = armatureData.boneDataSize,
                           });
            proposedFreeAlloc = armatureData.freeAllocations.end();

            free(armatureData.pBoneData);
            armatureData.pBoneData = pNewBoneData;
            armatureData.boneDataSize *= 2;
        }
    } while (proposedFreeAlloc == armatureData.freeAllocations.end());

    RenderSystemArmatureData::ArmatureBoneAlloc newEntry = *proposedFreeAlloc;
    if (newEntry.size == requiredBoneDataSize) {
        armatureData.freeAllocations.erase(proposedFreeAlloc);
    } else {
        newEntry.size = requiredBoneDataSize;

        proposedFreeAlloc->offset += requiredBoneDataSize;
        proposedFreeAlloc->size -= requiredBoneDataSize;
    }

    *pArmatureEntry = newEntry;
    armatureIndex = (pArmatureEntry - armatureData.pArmatureAllocations);

    return to_foeResult(FOE_BRINGUP_SUCCESS);
}

void clearArmatureData(RenderSystemArmatureData &armatureData, uint32_t &armatureIndex) {
    if (armatureIndex == UINT32_MAX)
        return;

    RenderSystemArmatureData::ArmatureBoneAlloc *pAllocation =
        armatureData.pArmatureAllocations + armatureIndex;

    freeAllocation(armatureData.freeAllocations, *pAllocation);

    *pAllocation = {};
    armatureIndex = UINT32_MAX;
}