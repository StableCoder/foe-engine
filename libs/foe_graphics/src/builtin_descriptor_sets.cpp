/*
    Copyright (C) 2020 George Cave.

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

#include <foe/graphics/builtin_descriptor_sets.hpp>

#include <foe/graphics/descriptor_set_layout_pool.hpp>
#include <vk_error_code.hpp>

#include "gfx_log.hpp"

#include <limits>

std::string to_string(foeBuiltinDescriptorSetLayoutFlagBits builtinSetLayout) {
    switch (builtinSetLayout) {
    case FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX:
        return "ProjectionViewMatrix";

    case FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_MODEL_MATRIX:
        return "ModelMatrix";

    case FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_BONE_STATE_MATRICES:
        return "BoneStateMatrices";

    default:
        return {};
    }
}

foeBuiltinDescriptorSetLayoutFlagBits to_builtin_set_layout(std::string_view str) {
    if (str == "ProjectionViewMatrix")
        return FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX;

    if (str == "ModelMatrix")
        return FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_MODEL_MATRIX;

    if (str == "BoneStateMatrices")
        return FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_BONE_STATE_MATRICES;

    return static_cast<foeBuiltinDescriptorSetLayoutFlagBits>(0);
}

auto foeBuiltinDescriptorSets::initialize(VkDevice device,
                                          foeDescriptorSetLayoutPool *pDescriptorSetLayoutPool)
    -> VkResult {
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
            FOE_LOG(Graphics, Error,
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
            FOE_LOG(Graphics, Error,
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
                Graphics, Error,
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

        VkResult res = vkCreateDescriptorPool(device, &poolCI, nullptr, &mDescriptorPool);
        if (res != VK_SUCCESS) {
            FOE_LOG(Graphics, Error,
                    "Failed to create foeBuiltinDescriptorSets descriptor pool with error: {}",
                    std::error_code{res}.message());
            return res;
        }
    }

    { // Dummy
        // Layout
        VkDescriptorSetLayoutCreateInfo layoutCI{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        };

        mDummyLayout = pDescriptorSetLayoutPool->get(&layoutCI);
        if (mDummyLayout == VK_NULL_HANDLE) {
            FOE_LOG(Graphics, Error, "Failed to create builtin descriptor set layout: Dummy");
            return VK_ERROR_INITIALIZATION_FAILED;
        }

        // Set
        VkDescriptorSetAllocateInfo setAI{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = mDescriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &mDummyLayout,
        };

        VkResult res = vkAllocateDescriptorSets(device, &setAI, &mDummySet);
        if (res != VK_SUCCESS) {
            FOE_LOG(Graphics, Error,
                    "Failed to allocate builtin descriptor set 'Dummy' with error: {}",
                    std::error_code{res}.message());
            return res;
        }
    }

    return VK_SUCCESS;
}

void foeBuiltinDescriptorSets::deinitialize(VkDevice device) {
    mDummySet = VK_NULL_HANDLE;
    mDummyLayout = VK_NULL_HANDLE;

    if (mDescriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(device, mDescriptorPool, nullptr);
    mDescriptorPool = VK_NULL_HANDLE;

    for (auto &it : mBuiltinLayouts) {
        it = VK_NULL_HANDLE;
    }
}

auto foeBuiltinDescriptorSets::getBuiltinLayout(
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

auto foeBuiltinDescriptorSets::getBuiltinSetLayoutIndex(
    foeBuiltinDescriptorSetLayoutFlags builtinLayout) const noexcept -> uint32_t {
    switch (builtinLayout) {
    case FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX:
        return ProjectionViewMatrix;
    case FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_MODEL_MATRIX:
        return ModelMatrix;
    case FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_BONE_STATE_MATRICES:
        return ModelAndBoneStateMatrices;
    default:
        FOE_LOG(Graphics, Warning,
                "Attempted to access unknown builtin descriptor set layout in "
                "getBuiltinSetLayoutIndex: {}",
                builtinLayout);
        return std::numeric_limits<uint32_t>::max();
    }
}

auto foeBuiltinDescriptorSets::getDummyLayout() const noexcept -> VkDescriptorSetLayout {
    return mDummyLayout;
}

auto foeBuiltinDescriptorSets::getDummySet() const noexcept -> VkDescriptorSet { return mDummySet; }