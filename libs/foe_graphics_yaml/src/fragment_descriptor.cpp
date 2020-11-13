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

#include <foe/graphics/yaml/fragment_descriptor.hpp>

#include <foe/graphics/fragment_descriptor.hpp>
#include <foe/graphics/fragment_descriptor_pool.hpp>
#include <foe/graphics/shader.hpp>
#include <foe/graphics/yaml/shader.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

bool yaml_read_fragment_descriptor(std::string const &nodeName,
                                   YAML::Node const &node,
                                   foeShaderPool *pShaderPool,
                                   foeFragmentDescriptorPool *pFragmentDescriptorPool,
                                   foeFragmentDescriptor **pFragmentDescriptor) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        // Rasterization
        VkPipelineRasterizationStateCreateInfo rasterizationSCI{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        };
        bool hasRasterization = yaml_read_optional("rasterization", subNode, rasterizationSCI);

        // Depth Stencil
        VkPipelineDepthStencilStateCreateInfo depthStencilSCI{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        };
        bool hasDepthStencil = yaml_read_optional("depth_stencil", subNode, depthStencilSCI);

        // Colour Blend
        bool hasColourBlend = false;
        std::vector<VkPipelineColorBlendAttachmentState> colourBlendAttachments;
        if (auto colourBlendNode = subNode["colour_blend_attachments"]; colourBlendNode) {
            for (auto it = colourBlendNode.begin(); it != colourBlendNode.end(); ++it) {
                VkPipelineColorBlendAttachmentState attachmentState;
                hasColourBlend |= yaml_read_required("", *it, attachmentState);

                colourBlendAttachments.emplace_back(attachmentState);
            }
        }
        VkPipelineColorBlendStateCreateInfo colourBlendSCI{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .attachmentCount = static_cast<uint32_t>(colourBlendAttachments.size()),
            .pAttachments = colourBlendAttachments.data(),
        };

        // Shaders
        foeShader *pFragShader = nullptr;
        yaml_read_shader("fragment_shader", subNode, pShaderPool, &pFragShader);

        // Get the descriptor
        auto *pNewFragmentDescriptor =
            pFragmentDescriptorPool->get(hasRasterization ? &rasterizationSCI : nullptr,
                                         hasDepthStencil ? &depthStencilSCI : nullptr,
                                         hasColourBlend ? &colourBlendSCI : nullptr, pFragShader);

        if (pNewFragmentDescriptor == nullptr) {
            throw foeYamlException(nodeName + " - Failed to general foeFragmentDescriptor");
        }

        *pFragmentDescriptor = pNewFragmentDescriptor;
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return true;
}

bool yaml_write_fragment_descriptor(std::string const &nodeName,
                                    foeFragmentDescriptor const *pFragmentDescriptor,
                                    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // Rasterization
        if (pFragmentDescriptor->hasRasterizationSCI)
            yaml_write_required("rasterization", pFragmentDescriptor->mRasterizationSCI, writeNode);

        // Depth Stencil
        if (pFragmentDescriptor->hasDepthStencilSCI)
            yaml_write_required("depth_stencil", pFragmentDescriptor->mDepthStencilSCI, writeNode);

        // Colour Blend
        if (pFragmentDescriptor->hasColourBlendSCI) {
            YAML::Node arrNode;
            for (uint32_t i = 0; i < pFragmentDescriptor->mColourBlendSCI.attachmentCount; ++i) {
                YAML::Node attachmentNode;
                yaml_write_required("", pFragmentDescriptor->mColourBlendAttachments[i],
                                    attachmentNode);

                arrNode.push_back(attachmentNode);
            }

            writeNode["colour_blend_attachments"] = arrNode;
        }

        // Fragment Shader
        yaml_write_shader("fragment_shader", pFragmentDescriptor->mFragment, writeNode);
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }

    return true;
}