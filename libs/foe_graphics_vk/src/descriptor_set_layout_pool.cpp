// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "descriptor_set_layout_pool.hpp"

#include "log.hpp"
#include "vk_result.h"

auto foeGfxVkDescriptorSetLayoutPool::initialize(VkDevice device) -> VkResult {
    if (mDevice != VK_NULL_HANDLE)
        return VK_ERROR_INITIALIZATION_FAILED;

    mDevice = device;

    return VK_SUCCESS;
}

void foeGfxVkDescriptorSetLayoutPool::deinitialize() {
    std::scoped_lock lock{mSync};

    for (auto &it : mLayouts) {
        if (it.layout != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(mDevice, it.layout, nullptr);
    }
    mLayouts.clear();

    mDevice = VK_NULL_HANDLE;
}

namespace {

bool compare_VkDescriptorSetLayoutBinding(VkDescriptorSetLayoutBinding const &lhs,
                                          VkDescriptorSetLayoutBinding const &rhs) {
    return (lhs.binding == rhs.binding) && (lhs.descriptorType == rhs.descriptorType) &&
           (lhs.descriptorCount == rhs.descriptorCount) && (lhs.stageFlags == rhs.stageFlags) &&
           (lhs.pImmutableSamplers == rhs.pImmutableSamplers);
}

} // namespace

auto foeGfxVkDescriptorSetLayoutPool::get(
    VkDescriptorSetLayoutCreateInfo const *pDescriptorSetLayoutCI) -> VkDescriptorSetLayout {
    std::scoped_lock lock{mSync};

    // Check if we already have this layout
    for (auto const &it : mLayouts) {
        if (pDescriptorSetLayoutCI->bindingCount != it.layoutCI.bindingCount ||
            pDescriptorSetLayoutCI->flags != it.layoutCI.flags)
            continue;

        bool same{true};
        for (uint32_t i = 0; i < it.layoutCI.bindingCount; ++i) {
            if (!compare_VkDescriptorSetLayoutBinding(pDescriptorSetLayoutCI->pBindings[i],
                                                      it.layoutBindings[i])) {
                same = false;
                break;
            }
        }

        if (same)
            return it.layout;
    }

    FOE_LOG(foeVkGraphics, FOE_LOG_LEVEL_VERBOSE, "Creating a new DescriptorSetLayout");

    VkDescriptorSetLayout newLayout;
    VkResult vkResult =
        vkCreateDescriptorSetLayout(mDevice, pDescriptorSetLayoutCI, nullptr, &newLayout);
    if (vkResult != VK_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        VkResultToString(vkResult, buffer);
        FOE_LOG(foeVkGraphics, FOE_LOG_LEVEL_ERROR,
                "vkCreateDescriptorSetLayout failed with error: {}", buffer);

        return VK_NULL_HANDLE;
    }

    Layout newEntry{
        .layout = newLayout,
        .layoutCI = *pDescriptorSetLayoutCI,
    };

    for (uint32_t i = 0; i < pDescriptorSetLayoutCI->bindingCount; ++i) {
        newEntry.layoutBindings.emplace_back(pDescriptorSetLayoutCI->pBindings[i]);
    }

    mLayouts.emplace_back(std::move(newEntry));

    return newLayout;
}

bool foeGfxVkDescriptorSetLayoutPool::getCI(
    VkDescriptorSetLayout layout,
    VkDescriptorSetLayoutCreateInfo &layoutCI,
    std::vector<VkDescriptorSetLayoutBinding> &layoutBindings) {
    std::scoped_lock lock{mSync};

    for (auto const &it : mLayouts) {
        if (layout == it.layout) {
            layoutCI = it.layoutCI;
            layoutCI.pBindings = nullptr;
            layoutBindings = it.layoutBindings;

            return true;
        }
    }

    return false;
}