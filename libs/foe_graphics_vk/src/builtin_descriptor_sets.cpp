// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "builtin_descriptor_sets.hpp"

#include "descriptor_set_layout_pool.hpp"
#include "log.hpp"
#include "vk_result.h"

auto foeGfxVkBuiltinDescriptorSets::initialize(
    VkDevice device, foeGfxVkDescriptorSetLayoutPool *pDescriptorSetLayoutPool) -> VkResult {
    { // ProjectionViewMatrix
        VkDescriptorSetLayoutBinding binding{
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        };

        VkDescriptorSetLayoutCreateInfo layoutCI{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = 1,
            .pBindings = &binding,
        };

        mBuiltinLayouts[0] = pDescriptorSetLayoutPool->get(&layoutCI);
        if (mBuiltinLayouts[0] == VK_NULL_HANDLE) {
            FOE_LOG(foeVkGraphics, FOE_LOG_LEVEL_ERROR,
                    "Failed to create the builtin descriptor set layout: ProjectionViewMatrix");
            return VK_ERROR_INITIALIZATION_FAILED;
        }
    }

    { // ModelMatrix
        VkDescriptorSetLayoutBinding binding{
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        };

        VkDescriptorSetLayoutCreateInfo layoutCI{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = 1,
            .pBindings = &binding,
        };

        mBuiltinLayouts[1] = pDescriptorSetLayoutPool->get(&layoutCI);
        if (mBuiltinLayouts[1] == VK_NULL_HANDLE) {
            FOE_LOG(foeVkGraphics, FOE_LOG_LEVEL_ERROR,
                    "Failed to create the builtin descriptor set layout: ModelMatrix");
            return VK_ERROR_INITIALIZATION_FAILED;
        }
    }

    { // ModelAndBoneStateMatrices
        std::array<VkDescriptorSetLayoutBinding, 2> bindings{
            VkDescriptorSetLayoutBinding{
                .binding = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            },
            VkDescriptorSetLayoutBinding{
                .binding = 2,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            },
        };

        VkDescriptorSetLayoutCreateInfo layoutCI{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings = bindings.data(),
        };

        mBuiltinLayouts[2] = pDescriptorSetLayoutPool->get(&layoutCI);
        if (mBuiltinLayouts[2] == VK_NULL_HANDLE) {
            FOE_LOG(
                foeVkGraphics, FOE_LOG_LEVEL_ERROR,
                "Failed to create the builtin descriptor set layout: ModelAndBoneStateMatrices");
            return VK_ERROR_INITIALIZATION_FAILED;
        }
    }

    { // Descriptor Pool
        VkDescriptorPoolSize size{
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
        };

        VkDescriptorPoolCreateInfo poolCI{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = 1,
            .poolSizeCount = 1,
            .pPoolSizes = &size,
        };

        VkResult vkResult = vkCreateDescriptorPool(device, &poolCI, nullptr, &mDescriptorPool);
        if (vkResult != VK_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            VkResultToString(vkResult, buffer);
            FOE_LOG(foeVkGraphics, FOE_LOG_LEVEL_ERROR,
                    "Failed to create foeGfxVkBuiltinDescriptorSets descriptor pool with error: {}",
                    buffer);

            return vkResult;
        }
    }

    { // Dummy
        // Layout
        VkDescriptorSetLayoutCreateInfo layoutCI{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        };

        mDummyLayout = pDescriptorSetLayoutPool->get(&layoutCI);
        if (mDummyLayout == VK_NULL_HANDLE) {
            FOE_LOG(foeVkGraphics, FOE_LOG_LEVEL_ERROR,
                    "Failed to create builtin descriptor set layout: Dummy");
            return VK_ERROR_INITIALIZATION_FAILED;
        }

        // Set
        VkDescriptorSetAllocateInfo setAI{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = mDescriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &mDummyLayout,
        };

        VkResult vkResult = vkAllocateDescriptorSets(device, &setAI, &mDummySet);
        if (vkResult != VK_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            VkResultToString(vkResult, buffer);
            FOE_LOG(foeVkGraphics, FOE_LOG_LEVEL_ERROR,
                    "Failed to allocate builtin descriptor set 'Dummy' with error: {}", buffer);

            return vkResult;
        }
    }

    return VK_SUCCESS;
}

void foeGfxVkBuiltinDescriptorSets::deinitialize(VkDevice device) {
    mDummySet = VK_NULL_HANDLE;
    mDummyLayout = VK_NULL_HANDLE;

    if (mDescriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(device, mDescriptorPool, nullptr);
    mDescriptorPool = VK_NULL_HANDLE;

    for (auto &it : mBuiltinLayouts) {
        it = VK_NULL_HANDLE;
    }
}

auto foeGfxVkBuiltinDescriptorSets::getBuiltinLayout(
    foeBuiltinDescriptorSetLayoutFlags builtinLayout) const noexcept -> VkDescriptorSetLayout {
    switch (builtinLayout) {
    case FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX:
        return mBuiltinLayouts[0];
    case FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_MODEL_MATRIX:
        return mBuiltinLayouts[1];
    case FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_BONE_STATE_MATRICES:
        return mBuiltinLayouts[2];
    default:
        return VK_NULL_HANDLE;
    }
}

auto foeGfxVkBuiltinDescriptorSets::getBuiltinSetLayoutIndex(
    foeBuiltinDescriptorSetLayoutFlags builtinLayout) const noexcept -> uint32_t {
    switch (builtinLayout) {
    case FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX:
        return ProjectionViewMatrix;
    case FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_MODEL_MATRIX:
        return ModelMatrix;
    case FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_BONE_STATE_MATRICES:
        return ModelAndBoneStateMatrices;
    default:
        FOE_LOG(foeVkGraphics, FOE_LOG_LEVEL_WARNING,
                "Attempted to access unknown builtin descriptor set layout in "
                "getBuiltinSetLayoutIndex: {}",
                builtinLayout);
        return std::numeric_limits<uint32_t>::max();
    }
}

auto foeGfxVkBuiltinDescriptorSets::getDummyLayout() const noexcept -> VkDescriptorSetLayout {
    return mDummyLayout;
}

auto foeGfxVkBuiltinDescriptorSets::getDummySet() const noexcept -> VkDescriptorSet {
    return mDummySet;
}