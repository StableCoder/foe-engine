// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "fragment_descriptor_pool.hpp"

#include <foe/graphics/vk/fragment_descriptor.h>

#include <cstdlib>

foeGfxVkFragmentDescriptorPoolImpl::~foeGfxVkFragmentDescriptorPoolImpl() {
    std::scoped_lock lock{mSync};

    for (auto pDescriptor : mDescriptors) {
        if (pDescriptor->pColourBlendAttachments)
            delete[] pDescriptor->pColourBlendAttachments;
        if (pDescriptor->pColourBlendSCI)
            delete pDescriptor->pColourBlendSCI;

        if (pDescriptor->pDepthStencilSCI)
            delete pDescriptor->pDepthStencilSCI;

        if (pDescriptor->pRasterizationSCI)
            delete pDescriptor->pRasterizationSCI;

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

auto foeGfxVkFragmentDescriptorPoolImpl::get(
    VkPipelineRasterizationStateCreateInfo const *pRasterizationSCI,
    VkPipelineDepthStencilStateCreateInfo const *pDepthStencilSCI,
    VkPipelineColorBlendStateCreateInfo const *pColourBlendSCI,
    foeGfxShader fragment) -> foeGfxVkFragmentDescriptor * {
    std::scoped_lock lock{mSync};

    for (auto *fragDescriptor : mDescriptors) {
        if (fragDescriptor->mFragment != fragment)
            continue;

        if ((pRasterizationSCI == nullptr && fragDescriptor->pRasterizationSCI != nullptr) ||
            (pRasterizationSCI != nullptr && fragDescriptor->pRasterizationSCI == nullptr) ||
            (pRasterizationSCI != nullptr &&
             !compare_VkPipelineRasterizationStateCreateInfo(*pRasterizationSCI,
                                                             *fragDescriptor->pRasterizationSCI)))
            continue;

        if ((pDepthStencilSCI == nullptr && fragDescriptor->pDepthStencilSCI != nullptr) ||
            (pDepthStencilSCI != nullptr && fragDescriptor->pDepthStencilSCI == nullptr) ||
            (pDepthStencilSCI != nullptr &&
             !compare_VkPipelineDepthStencilStateCreateInfo(*pDepthStencilSCI,
                                                            *fragDescriptor->pDepthStencilSCI)))
            continue;

        if ((pColourBlendSCI == nullptr && fragDescriptor->pColourBlendSCI != nullptr) ||
            (pColourBlendSCI != nullptr && fragDescriptor->pColourBlendSCI == nullptr) ||
            (pColourBlendSCI != nullptr && !compare_VkPipelineColorBlendStateCreateInfo(
                                               *pColourBlendSCI, *fragDescriptor->pColourBlendSCI)))
            continue;

        return fragDescriptor;
    }

    auto pFragDescriptor = new (std::nothrow) foeGfxVkFragmentDescriptor;
    if (pFragDescriptor == nullptr)
        std::abort();

    { // Set fragment descriptor data
        *pFragDescriptor = {
            .mFragment = fragment,
        };

        if (pRasterizationSCI)
            pFragDescriptor->pRasterizationSCI =
                new VkPipelineRasterizationStateCreateInfo{*pRasterizationSCI};

        if (pDepthStencilSCI)
            pFragDescriptor->pDepthStencilSCI =
                new VkPipelineDepthStencilStateCreateInfo{*pDepthStencilSCI};

        if (pColourBlendSCI) {
            pFragDescriptor->pColourBlendSCI =
                new VkPipelineColorBlendStateCreateInfo{*pColourBlendSCI};

            pFragDescriptor->pColourBlendAttachments =
                new VkPipelineColorBlendAttachmentState[pColourBlendSCI->attachmentCount];

            std::copy(pColourBlendSCI->pAttachments,
                      pColourBlendSCI->pAttachments + pColourBlendSCI->attachmentCount,
                      pFragDescriptor->pColourBlendAttachments);
            pFragDescriptor->pColourBlendSCI->pAttachments =
                pFragDescriptor->pColourBlendAttachments;
        }
    }

    mDescriptors.emplace_back(pFragDescriptor);

    return pFragDescriptor;
}