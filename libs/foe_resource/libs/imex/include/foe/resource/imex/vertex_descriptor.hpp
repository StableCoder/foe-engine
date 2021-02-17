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

#ifndef FOE_RESOURCE_IMEX_VERTEX_DESCRIPTOR_HPP
#define FOE_RESOURCE_IMEX_VERTEX_DESCRIPTOR_HPP

#include <foe/resource/imex/export.h>
#include <foe/resource/vertex_descriptor.hpp>

#include <string>
#include <string_view>
#include <vector>

FOE_RES_IMEX_EXPORT bool import_vertex_descriptor_definition(
    std::string_view vertexDescriptorName,
    std::string &vertexShader,
    std::string &tessellationControlShader,
    std::string &tessellationEvaluationShader,
    std::string &geometryShader,
    VkPipelineVertexInputStateCreateInfo &vertexInputSCI,
    std::vector<VkVertexInputBindingDescription> &inputBindings,
    std::vector<VkVertexInputAttributeDescription> &inputAttributes,
    VkPipelineInputAssemblyStateCreateInfo &inputAssemblySCI,
    VkPipelineTessellationStateCreateInfo &tessellationSCI);

FOE_RES_IMEX_EXPORT bool export_vertex_descriptor_definition(
    foeVertexDescriptor const *pVertexDescriptor);

#endif // FOE_RESOURCE_IMEX_VERTEX_DESCRIPTOR_HPP