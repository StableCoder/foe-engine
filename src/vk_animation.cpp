#include "vk_animation.hpp"

#include <foe/graphics/vk/session.hpp>
#include <glm/glm.hpp>

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

VkResult VkAnimationPool::generateBoneAnimation(uint32_t frameIndex,
                                                double time,
                                                std::vector<foeMeshBone> const &bones,
                                                foeAnimation const *pAnimation) {
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

    if (boneUniform.capacity < bones.size()) {
        VkBufferCreateInfo bufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = sizeof(glm::mat4) * bones.size(),
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

        boneUniform.capacity = bones.size();
    }

    vkResetDescriptorPool(mDevice, mBoneDescriptorPools[frameIndex], 0);

    glm::mat4 *pBufferData;
    VkDeviceSize offset = 0;
    vmaMapMemory(mAllocator, boneUniform.alloc, reinterpret_cast<void **>(&pBufferData));

    if (pAnimation == nullptr) {
        // No animation, oops
        for (int i = 0; i < bones.size(); ++i) {
            *pBufferData = glm::mat4(1.f) * bones[i].offsetMatrix;
            pBufferData++;
        }
    } else {
        auto totalDuration = pAnimation->duration / pAnimation->ticksPerSecond;
        while (time > totalDuration) {
            time -= totalDuration;
        }

        glm::mat4 transform(1.f);
        auto animationTime = time * pAnimation->ticksPerSecond;
        for (int i = 0; i < bones.size(); ++i) {
            foeMeshBone const &bone = bones[i];
            foeNodeAnimationChannel const *pChannel{nullptr};
            for (size_t i = 0; i < pAnimation->nodeChannels.size(); ++i) {
                if (pAnimation->nodeChannels[i].nodeName == bone.name) {
                    pChannel = &pAnimation->nodeChannels[i];
                }
            }

            if (pChannel != nullptr &&
                (!pChannel->positionKeys.empty() || !pChannel->rotationKeys.empty() ||
                 !pChannel->scalingKeys.empty())) {
                // Animation channel isn't empty
                glm::vec3 posVec = interpolatePosition(animationTime, pChannel);
                glm::mat4 posMat = glm::translate(glm::mat4(1.f), posVec);

                glm::quat rotQuat = interpolateRotation(animationTime, pChannel);
                glm::mat4 rotMat = glm::mat4_cast(rotQuat);

                glm::vec3 scaleVec = interpolateScaling(animationTime, pChannel);
                glm::mat4 scaleMat = glm::scale(glm::mat4(1.f), scaleVec);

                transform = transform * posMat * rotMat * scaleMat;
            } else {
                transform = glm::mat4(1.f);
            }

            *pBufferData = transform * bones[i].offsetMatrix;
            pBufferData++;
        }
    }

    { // Descriptor Set
        VkDescriptorSetAllocateInfo setAI{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = mBoneDescriptorPools[frameIndex],
            .descriptorSetCount = 1U,
            .pSetLayouts = &mBoneSetLayout,
        };

        res = vkAllocateDescriptorSets(mDevice, &setAI, &mSet);
        if (res != VK_SUCCESS) {
            return res;
        }

        VkDescriptorBufferInfo bufferInfo{
            .buffer = boneUniform.buffer,
            .offset = offset,
            .range = sizeof(glm::mat4) * bones.size(),
        };

        VkWriteDescriptorSet writeSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = mSet,
            .dstBinding = mBoneSetBinding,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &bufferInfo,
        };

        vkUpdateDescriptorSets(mDevice, 1, &writeSet, 0, nullptr);
    }

    vmaUnmapMemory(mAllocator, boneUniform.alloc);

    return res;
}
