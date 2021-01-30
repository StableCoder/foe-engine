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

#ifndef FOE_GRAPHICS_VERTEX_DESCRIPTOR_HPP
#define FOE_GRAPHICS_VERTEX_DESCRIPTOR_HPP

#include <foe/graphics/shader.hpp>
#include <vulkan/vulkan.h>

#include <vector>

struct foeVertexDescriptor {
    auto getBuiltinSetLayouts() const noexcept -> foeBuiltinDescriptorSetLayoutFlags {
        foeBuiltinDescriptorSetLayoutFlags flags = 0;

        if (mVertex != nullptr)
            flags |= foeGfxShaderGetBuiltinDescriptorSetLayouts(mVertex);

        if (mTessellationControl != nullptr)
            flags |= foeGfxShaderGetBuiltinDescriptorSetLayouts(mTessellationControl);

        if (mTessellationEvaluation != nullptr)
            flags |= foeGfxShaderGetBuiltinDescriptorSetLayouts(mTessellationEvaluation);

        if (mGeometry != nullptr)
            flags |= foeGfxShaderGetBuiltinDescriptorSetLayouts(mGeometry);

        return flags;
    }

    auto getVertexInputSCI() noexcept -> VkPipelineVertexInputStateCreateInfo const * {
        mVertexInputSCI.vertexBindingDescriptionCount = mVertexInputBindings.size();
        mVertexInputSCI.pVertexBindingDescriptions = mVertexInputBindings.data();

        mVertexInputSCI.vertexAttributeDescriptionCount = mVertexInputAttributes.size();
        mVertexInputSCI.pVertexAttributeDescriptions = mVertexInputAttributes.data();

        return &mVertexInputSCI;
    }

    auto getTessellationSCI() const noexcept -> VkPipelineTessellationStateCreateInfo const * {
        if (mTessellationControl != nullptr || mTessellationEvaluation != nullptr) {
            return &mTessellationSCI;
        }

        return nullptr;
    }

    foeGfxShader mVertex{FOE_NULL_HANDLE};
    foeGfxShader mTessellationControl{FOE_NULL_HANDLE};
    foeGfxShader mTessellationEvaluation{FOE_NULL_HANDLE};
    foeGfxShader mGeometry{FOE_NULL_HANDLE};

    std::vector<VkVertexInputBindingDescription> mVertexInputBindings;
    std::vector<VkVertexInputAttributeDescription> mVertexInputAttributes;
    VkPipelineVertexInputStateCreateInfo mVertexInputSCI{};

    VkPipelineInputAssemblyStateCreateInfo mInputAssemblySCI{};

    VkPipelineTessellationStateCreateInfo mTessellationSCI{};
};

#endif // FOE_GRAPHICS_VERTEX_DESCRIPTOR_HPP