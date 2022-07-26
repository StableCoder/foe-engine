// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VERTEX_DESCRIPTOR_HPP
#define FOE_GRAPHICS_VERTEX_DESCRIPTOR_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/shader.h>
#include <vulkan/vulkan.h>

struct foeGfxVertexDescriptor {
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
        mVertexInputSCI.vertexBindingDescriptionCount = vertexInputBindingCount;
        mVertexInputSCI.pVertexBindingDescriptions = pVertexInputBindings;

        mVertexInputSCI.vertexAttributeDescriptionCount = vertexInputAttributeCount;
        mVertexInputSCI.pVertexAttributeDescriptions = pVertexInputAttributes;

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

    uint32_t vertexInputBindingCount{0};
    VkVertexInputBindingDescription *pVertexInputBindings{nullptr};
    uint32_t vertexInputAttributeCount{0};
    VkVertexInputAttributeDescription *pVertexInputAttributes{nullptr};
    VkPipelineVertexInputStateCreateInfo mVertexInputSCI{};

    VkPipelineInputAssemblyStateCreateInfo mInputAssemblySCI{};

    VkPipelineTessellationStateCreateInfo mTessellationSCI{};
};

FOE_GFX_EXPORT void cleanup_foeGfxVertexDescriptor(foeGfxVertexDescriptor *pData);

#endif // FOE_GRAPHICS_VERTEX_DESCRIPTOR_HPP