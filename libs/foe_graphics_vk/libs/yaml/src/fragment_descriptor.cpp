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