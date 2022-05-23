/*
    Copyright (C) 2020-2022 George Cave.

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

#ifndef FOE_GRAPHICS_FRAGMENT_DESCRIPTOR_POOL_HPP
#define FOE_GRAPHICS_FRAGMENT_DESCRIPTOR_POOL_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/shader.h>
#include <vulkan/vulkan.h>

#include <mutex>
#include <vector>

struct foeGfxVkFragmentDescriptor;

class foeGfxVkFragmentDescriptorPool {
  public:
    FOE_GFX_EXPORT ~foeGfxVkFragmentDescriptorPool();

    FOE_GFX_EXPORT auto get(VkPipelineRasterizationStateCreateInfo const *pRasterizationSCI,
                            VkPipelineDepthStencilStateCreateInfo const *pDepthStencilSCI,
                            VkPipelineColorBlendStateCreateInfo const *pColourBlendSCI,
                            foeGfxShader fragment) -> foeGfxVkFragmentDescriptor *;

  private:
    std::mutex mSync;
    std::vector<foeGfxVkFragmentDescriptor *> mDescriptors;
};

#endif // FOE_GRAPHICS_FRAGMENT_DESCRIPTOR_POOL_HPP