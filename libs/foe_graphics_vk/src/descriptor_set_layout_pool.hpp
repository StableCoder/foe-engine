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

#ifndef DESCRIPTOR_SET_LAYOUT_POOL_HPP
#define DESCRIPTOR_SET_LAYOUT_POOL_HPP

#include <foe/graphics/export.h>
#include <vulkan/vulkan.h>

#include <mutex>
#include <vector>

/// Handles generation and retrieval of requested DescriptorSetLayouts.
class foeGfxVkDescriptorSetLayoutPool {
  public:
    /** Initializes the pool with the graphics backend
     * @param device Vulkan device handle of the graphics environment
     * @return VK_SUCCESS on success. An appropriate error code otherwise.
     */
    auto initialize(VkDevice device) -> VkResult;

    /** Deinitialized the object
     *
     * All generated DescriptorSetLayouts are destroyed and become unusable.
     */
    void deinitialize();

    /** Attempts to return a valid DescriptorSetLayout from the given create info
     * @param pDescriptorSetLayoutCI Descripts the set layout to get a handle for
     * @return A valid handle, or VK_NULL_HANDLE if it failed to find/generate one.
     */
    auto get(VkDescriptorSetLayoutCreateInfo const *pDescriptorSetLayoutCI)
        -> VkDescriptorSetLayout;

    bool getCI(VkDescriptorSetLayout layout,
               VkDescriptorSetLayoutCreateInfo &layoutCI,
               std::vector<VkDescriptorSetLayoutBinding> &layoutBindings);

  private:
    struct Layout {
        VkDescriptorSetLayout layout;
        VkDescriptorSetLayoutCreateInfo layoutCI;
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    };

    VkDevice mDevice{VK_NULL_HANDLE};

    std::mutex mSync;
    std::vector<Layout> mLayouts;
};

#endif // DESCRIPTOR_SET_LAYOUT_POOL_HPP