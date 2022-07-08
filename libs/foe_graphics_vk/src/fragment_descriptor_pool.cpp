// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/fragment_descriptor_pool.hpp>

#include <foe/graphics/vk/fragment_descriptor.hpp>

foeGfxVkFragmentDescriptorPool::~foeGfxVkFragmentDescriptorPool() {
    std::scoped_lock lock{mSync};

    for (auto pDescriptor : mDescriptors) {
        delete pDescriptor;
    }
}

namespace {

bool compare_VkPipelineRasterizationStateCreateInfo(
    VkPipelineRasterizationStateCreateInfo const &lhs,
    VkPipelineRasterizationStateCreateInfo const &rhs) {
    return (lhs.flags == rhs.flags) && (lhs.depthClampEnable == rhs.depthClampEnable) &&
           (lhs.rasterizerDiscardEnable == rhs.rasterizerDiscardEnable) &&
           (lhs.polygonMode == rhs.polygonMode) && (lhs.cullMode == rhs.cullMode) &&
           (lhs.frontFace == rhs.frontFace) && (lhs.depthBiasEnable == rhs.depthBiasEnable) &&
           (lhs.depthBiasConstantFactor == rhs.depthBiasConstantFactor) &&
           (lhs.depthBiasClamp == rhs.depthBiasClamp) &&
           (lhs.depthBiasSlopeFactor == rhs.depthBiasSlopeFactor) &&
           (lhs.lineWidth == rhs.lineWidth);
}

bool compare_VkStencilOpState(VkStencilOpState const &lhs, VkStencilOpState const &rhs) noexcept {
    return (lhs.failOp == rhs.failOp) && (lhs.passOp == rhs.passOp) &&
           (lhs.depthFailOp == rhs.depthFailOp) && (lhs.compareOp == rhs.compareOp) &&
           (lhs.compareMask == rhs.compareMask) && (lhs.writeMask == rhs.writeMask) &&
           (lhs.reference == rhs.reference);
}

bool compare_VkPipelineDepthStencilStateCreateInfo(
    VkPipelineDepthStencilStateCreateInfo const &lhs,
    VkPipelineDepthStencilStateCreateInfo const &rhs) noexcept {
    return (lhs.flags == rhs.flags) && (lhs.depthTestEnable == rhs.depthTestEnable) &&
           (lhs.depthWriteEnable == rhs.depthWriteEnable) &&
           (lhs.depthCompareOp == rhs.depthCompareOp) &&
           (lhs.depthBoundsTestEnable == rhs.depthBoundsTestEnable) &&
           (lhs.stencilTestEnable == rhs.stencilTestEnable) &&
           (compare_VkStencilOpState(lhs.front, rhs.front)) &&
           (compare_VkStencilOpState(lhs.back, rhs.back)) &&
           (lhs.minDepthBounds == rhs.minDepthBounds) && (lhs.maxDepthBounds == rhs.maxDepthBounds);
}

bool compare_VkPipelineColorBlendAttachmentState(
    VkPipelineColorBlendAttachmentState const &lhs,
    VkPipelineColorBlendAttachmentState const &rhs) noexcept {
    return (lhs.blendEnable == rhs.blendEnable) &&
           (lhs.srcColorBlendFactor == rhs.srcColorBlendFactor) &&
           (lhs.dstColorBlendFactor == rhs.dstColorBlendFactor) &&
           (lhs.colorBlendOp == rhs.colorBlendOp) &&
           (lhs.srcAlphaBlendFactor == rhs.srcAlphaBlendFactor) &&
           (lhs.dstAlphaBlendFactor == rhs.dstAlphaBlendFactor) &&
           (lhs.alphaBlendOp == rhs.alphaBlendOp) && (lhs.colorWriteMask == rhs.colorWriteMask);
}

bool compare_VkPipelineColorBlendStateCreateInfo(
    VkPipelineColorBlendStateCreateInfo const &lhs,
    VkPipelineColorBlendStateCreateInfo const &rhs) noexcept {
    if (lhs.attachmentCount != rhs.attachmentCount)
        return false;

    for (uint32_t i = 0; i < lhs.attachmentCount; ++i) {
        if (!compare_VkPipelineColorBlendAttachmentState(lhs.pAttachments[i], rhs.pAttachments[i]))
            return false;
    }

    for (uint32_t i = 0; i < 4; ++i) {
        if (lhs.blendConstants[i] != rhs.blendConstants[i])
            return false;
    }

    return (lhs.sType == rhs.sType) && (lhs.flags == rhs.flags) &&
           (lhs.logicOpEnable == rhs.logicOpEnable) && (lhs.logicOp == rhs.logicOp);
}

} // namespace

auto foeGfxVkFragmentDescriptorPool::get(
    VkPipelineRasterizationStateCreateInfo const *pRasterizationSCI,
    VkPipelineDepthStencilStateCreateInfo const *pDepthStencilSCI,
    VkPipelineColorBlendStateCreateInfo const *pColourBlendSCI,
    foeGfxShader fragment) -> foeGfxVkFragmentDescriptor * {
    std::scoped_lock lock{mSync};

    for (auto *fragDescriptor : mDescriptors) {
        if (fragDescriptor->mFragment != fragment)
            continue;

        if ((pRasterizationSCI == nullptr && fragDescriptor->hasRasterizationSCI) ||
            (pRasterizationSCI != nullptr && !fragDescriptor->hasRasterizationSCI) ||
            (pRasterizationSCI != nullptr &&
             !compare_VkPipelineRasterizationStateCreateInfo(*pRasterizationSCI,
                                                             fragDescriptor->mRasterizationSCI)))
            continue;

        if ((pDepthStencilSCI == nullptr && fragDescriptor->hasDepthStencilSCI) ||
            (pDepthStencilSCI != nullptr && !fragDescriptor->hasDepthStencilSCI) ||
            (pDepthStencilSCI != nullptr &&
             !compare_VkPipelineDepthStencilStateCreateInfo(*pDepthStencilSCI,
                                                            fragDescriptor->mDepthStencilSCI)))
            continue;

        if ((pColourBlendSCI == nullptr && fragDescriptor->hasColourBlendSCI) ||
            (pColourBlendSCI != nullptr && !fragDescriptor->hasColourBlendSCI) ||
            (pColourBlendSCI != nullptr && !compare_VkPipelineColorBlendStateCreateInfo(
                                               *pColourBlendSCI, fragDescriptor->mColourBlendSCI)))
            continue;

        return fragDescriptor;
    }

    auto pFragDescriptor = new foeGfxVkFragmentDescriptor(pRasterizationSCI, pDepthStencilSCI,
                                                          pColourBlendSCI, fragment);

    mDescriptors.emplace_back(pFragDescriptor);

    return pFragDescriptor;
}