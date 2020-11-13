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

#include <foe/graphics/descriptor_set_layout_pool.hpp>

#include <vk_equality_checks.hpp>
#include <vk_error_code.hpp>

#include "gfx_log.hpp"

auto foeDescriptorSetLayoutPool::initialize(VkDevice device) -> VkResult {
    if (mDevice != VK_NULL_HANDLE)
        return VK_ERROR_INITIALIZATION_FAILED;

    mDevice = device;

    return VK_SUCCESS;
}

void foeDescriptorSetLayoutPool::deinitialize() {
    std::scoped_lock lock{mSync};

    for (auto &it : mLayouts) {
        if (it.layout != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(mDevice, it.layout, nullptr);
    }
    mLayouts.clear();

    mDevice = VK_NULL_HANDLE;
}

auto foeDescriptorSetLayoutPool::get(VkDescriptorSetLayoutCreateInfo const *pDescriptorSetLayoutCI)
    -> VkDescriptorSetLayout {
    std::scoped_lock lock{mSync};

    // Check if we already have this layout
    for (auto const &it : mLayouts) {
        if (pDescriptorSetLayoutCI->bindingCount != it.layoutCI.bindingCount ||
            pDescriptorSetLayoutCI->flags != it.layoutCI.flags)
            continue;

        bool same{true};
        for (uint32_t i = 0; i < it.layoutCI.bindingCount; ++i) {
            if (pDescriptorSetLayoutCI->pBindings[i] != it.layoutBindings[i]) {
                same = false;
                break;
            }
        }

        if (same)
            return it.layout;
    }

    FOE_LOG(Graphics, Verbose, "Creating a new DescriptorSetLayout");

    VkDescriptorSetLayout newLayout;
    std::error_code errc =
        vkCreateDescriptorSetLayout(mDevice, pDescriptorSetLayoutCI, nullptr, &newLayout);
    if (errc) {
        FOE_LOG(Graphics, Error, "vkCreateDescriptorSetLayout failed with error: {}",
                errc.message());
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

bool foeDescriptorSetLayoutPool::getCI(VkDescriptorSetLayout layout,
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