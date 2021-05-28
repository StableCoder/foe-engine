#include "vk_animation.hpp"

#include <foe/graphics/vk/session.hpp>
#include <foe/resource/armature.hpp>
#include <foe/resource/armature_pool.hpp>
#include <foe/resource/mesh.hpp>
#include <foe/resource/mesh_pool.hpp>
#include <glm/glm.hpp>

#include "armature_state.hpp"
#include "render_state.hpp"

VkResult VkAnimationPool::initialize(foeGfxSession gfxSession,
                                     VkDescriptorSetLayout boneSetLayout,
                                     uint32_t boneSetBinding) {
    VkResult res;

    mDevice = foeGfxVkGetDevice(gfxSession);
    mAllocator = foeGfxVkGetAllocator(gfxSession);

    mBoneSetLayout = boneSetLayout;
    mBoneSetBinding = boneSetBinding;

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

void VkAnimationPool::deinitialize() {
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

    mAllocator = VK_NULL_HANDLE;
    mDevice = VK_NULL_HANDLE;
}

VkResult VkAnimationPool::uploadBoneOffsets(uint32_t frameIndex,
                                            foeArmatureStatePool *pArmatureStatePool,
                                            foeRenderStatePool *pRenderStatePool,
                                            foeArmaturePool *pArmaturePool,
                                            foeMeshPool *pMeshPool) {
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

    auto *pID = pRenderStatePool->begin();
    auto const *pEndID = pRenderStatePool->end();
    auto *pRenderState = pRenderStatePool->begin<1>();

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

        auto searchOffset = pArmatureStatePool->find(*pID);
        if (searchOffset != pArmatureStatePool->size()) {
            pArmatureState = pArmatureStatePool->begin<1>() + searchOffset;
        } else {
            continue;
        }

        foeMesh *pMesh = pMeshPool->find(pRenderState->mesh);
        foeArmature *pArmature = pArmaturePool->find(pArmatureState->armatureID);

        if (pMesh == nullptr || pArmature == nullptr ||
            pMesh->getLoadState() != foeResourceLoadState::Loaded ||
            pArmature->getLoadState() != foeResourceLoadState::Loaded ||
            pArmatureState->armatureState.empty()) {
            continue;
        }

        glm::mat4 lastBone = glm::mat4{1.f};
        glm::mat4 transform(1.f);
        for (auto const &bone : pMesh->data.gfxBones) {
            // Find the matching armature node, if it exists
            foeArmatureNode *pArmatureNode{nullptr};
            size_t armatureNodeIndex;
            for (size_t i = 0; i < pArmature->data.armature.size(); ++i) {
                if (pArmature->data.armature[i].name == bone.name) {
                    pArmatureNode = &pArmature->data.armature[i];
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