/*
    Copyright (C) 2021 George Cave.

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

#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

bool yaml_write_gfx_fragment_descriptor(std::string const &nodeName,
                                        foeGfxVkFragmentDescriptor const *pFragmentDescriptor,
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
            yaml_write_optional("colour_blend", VkPipelineColorBlendStateCreateInfo{},
                                pFragmentDescriptor->mColourBlendSCI, writeNode);

            /// @todo Implement YAML parsing for
            /// VkPipelineColorBlendStateCreateInfo::blendConstants[4]

            // Attachments
            YAML::Node arrNode;
            for (uint32_t i = 0; i < pFragmentDescriptor->mColourBlendSCI.attachmentCount; ++i) {
                YAML::Node attachmentNode;
                yaml_write_required("", pFragmentDescriptor->mColourBlendAttachments[i],
                                    attachmentNode);

                arrNode.push_back(attachmentNode);
            }

            writeNode["colour_blend_attachments"] = arrNode;
        }
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

bool yaml_read_gfx_fragment_descriptor(
    std::string const &nodeName,
    YAML::Node const &node,
    bool &hasRasterizationSCI,
    VkPipelineRasterizationStateCreateInfo &rasterizationSCI,
    bool &hasDepthStencilSCI,
    VkPipelineDepthStencilStateCreateInfo &depthStencilSCI,
    bool &hasColourBlendSCI,
    VkPipelineColorBlendStateCreateInfo &colourBlendSCI,
    std::vector<VkPipelineColorBlendAttachmentState> &colourBlendAttachments) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        // Rasterization
        rasterizationSCI = VkPipelineRasterizationStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        };
        hasRasterizationSCI = yaml_read_optional("rasterization", subNode, rasterizationSCI);

        // Depth Stencil
        depthStencilSCI = VkPipelineDepthStencilStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        };
        hasDepthStencilSCI = yaml_read_optional("depth_stencil", subNode, depthStencilSCI);

        // Colour Blend
        colourBlendSCI = VkPipelineColorBlendStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        };
        hasColourBlendSCI = yaml_read_optional("colour_blend", subNode, colourBlendSCI);

        /// @todo Implement YAML parsing for VkPipelineColorBlendStateCreateInfo::blendConstants[4]

        colourBlendAttachments.clear();
        if (auto colourBlendNode = subNode["colour_blend_attachments"]; colourBlendNode) {
            hasColourBlendSCI = true;
            for (auto it = colourBlendNode.begin(); it != colourBlendNode.end(); ++it) {
                VkPipelineColorBlendAttachmentState attachmentState;
                yaml_read_required("", *it, attachmentState);

                colourBlendAttachments.emplace_back(attachmentState);
            }
        }
        colourBlendSCI.attachmentCount = static_cast<uint32_t>(colourBlendAttachments.size());
        colourBlendSCI.pAttachments = colourBlendAttachments.data();
    } catch (foeYamlException const &e) {
        throw foeYamlException(nodeName + "::" + e.what());
    }

    return true;
}