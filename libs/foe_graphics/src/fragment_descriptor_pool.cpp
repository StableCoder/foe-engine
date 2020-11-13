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

#include <foe/graphics/fragment_descriptor_pool.hpp>

#include <foe/graphics/fragment_descriptor.hpp>
#include <vk_equality_checks.hpp>

auto foeFragmentDescriptorPool::get(VkPipelineRasterizationStateCreateInfo const *pRasterizationSCI,
                                    VkPipelineDepthStencilStateCreateInfo const *pDepthStencilSCI,
                                    VkPipelineColorBlendStateCreateInfo const *pColourBlendSCI,
                                    foeShader *pFragment) -> foeFragmentDescriptor * {
    std::scoped_lock lock{mSync};

    for (auto *fragDescriptor : mDescriptors) {
        if (fragDescriptor->mFragment != pFragment)
            continue;

        if ((pRasterizationSCI == nullptr && fragDescriptor->hasRasterizationSCI) ||
            (pRasterizationSCI != nullptr && !fragDescriptor->hasRasterizationSCI) ||
            (pRasterizationSCI != nullptr &&
             *pRasterizationSCI != fragDescriptor->mRasterizationSCI))
            continue;

        if ((pDepthStencilSCI == nullptr && fragDescriptor->hasDepthStencilSCI) ||
            (pDepthStencilSCI != nullptr && !fragDescriptor->hasDepthStencilSCI) ||
            (pDepthStencilSCI != nullptr && *pDepthStencilSCI != fragDescriptor->mDepthStencilSCI))
            continue;

        if ((pColourBlendSCI == nullptr && fragDescriptor->hasColourBlendSCI) ||
            (pColourBlendSCI != nullptr && !fragDescriptor->hasColourBlendSCI) ||
            (pColourBlendSCI != nullptr && *pColourBlendSCI != fragDescriptor->mColourBlendSCI))
            continue;

        return fragDescriptor;
    }

    auto pFragDescriptor =
        new foeFragmentDescriptor(pRasterizationSCI, pDepthStencilSCI, pColourBlendSCI, pFragment);

    mDescriptors.emplace_back(pFragDescriptor);

    return pFragDescriptor;
}