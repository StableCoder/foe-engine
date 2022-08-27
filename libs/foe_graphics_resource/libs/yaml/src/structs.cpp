// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/yaml/structs.hpp>

#include <foe/ecs/yaml/id.hpp>
#include <foe/graphics/resource/cleanup.h>
#include <foe/graphics/resource/image_create_info.h>
#include <foe/graphics/resource/material_create_info.h>
#include <foe/graphics/resource/mesh_create_info.h>
#include <foe/graphics/resource/shader_create_info.h>
#include <foe/graphics/resource/vertex_descriptor_create_info.h>
#include <foe/graphics/vk/compare.h>
#include <foe/graphics/vk/yaml/structs.hpp>
#include <foe/graphics/vk/yaml/vk_structs.hpp>
#include <foe/model/assimp/flags.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/glm.hpp>
#include <foe/yaml/pod.hpp>
#include <vk_struct_compare.h>

#include "assimp_flags.hpp"

bool yaml_read_foeImageCreateInfo(std::string const &nodeName,
                                  YAML::Node const &node,
                                  foeImageCreateInfo &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    foeImageCreateInfo newData = {};
    try {
        // char const * - pFile[null-terminated]
        if (std::string pFile; yaml_read_string("file", readNode, pFile)) {
            newData.pFile = (char *)malloc(pFile.size() + 1);
            memcpy((char *)newData.pFile, pFile.c_str(), pFile.size() + 1);
        }
    } catch (foeYamlException const &e) {
        cleanup_foeImageCreateInfo(&newData);

        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_foeImageCreateInfo(std::string const &nodeName,
                                   foeImageCreateInfo const &data,
                                   YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // char const * - pFile[null-terminated]
        if (data.pFile) {
            yaml_write_string("file", data.pFile, writeNode);
        }
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

bool yaml_read_foeMaterialCreateInfo(std::string const &nodeName,
                                     YAML::Node const &node,
                                     foeEcsGroupTranslator groupTranslator,
                                     foeMaterialCreateInfo &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    foeMaterialCreateInfo newData = {};
    try {
        // foeResourceID - fragmentShader
        yaml_read_foeResourceID("fragment_shader", readNode, groupTranslator,
                                newData.fragmentShader);

        // foeResourceID - image
        yaml_read_foeResourceID("image", readNode, groupTranslator, newData.image);

        // VkPipelineRasterizationStateCreateInfo* - pRasterizationSCI
        if (VkPipelineRasterizationStateCreateInfo rasterization_sci = {};
            yaml_read_VkPipelineRasterizationStateCreateInfo("rasterization_sci", readNode,
                                                             rasterization_sci)) {
            newData.pRasterizationSCI = (VkPipelineRasterizationStateCreateInfo *)malloc(
                sizeof(VkPipelineRasterizationStateCreateInfo));
            *newData.pRasterizationSCI = rasterization_sci;
        }

        // VkPipelineDepthStencilStateCreateInfo* - pDepthStencilSCI
        if (VkPipelineDepthStencilStateCreateInfo depth_stencil_sci = {};
            yaml_read_VkPipelineDepthStencilStateCreateInfo("depth_stencil_sci", readNode,
                                                            depth_stencil_sci)) {
            newData.pDepthStencilSCI = (VkPipelineDepthStencilStateCreateInfo *)malloc(
                sizeof(VkPipelineDepthStencilStateCreateInfo));
            *newData.pDepthStencilSCI = depth_stencil_sci;
        }

        // VkPipelineColorBlendStateCreateInfo* - pColourBlendSCI
        if (VkPipelineColorBlendStateCreateInfo colour_blend_sci = {};
            yaml_read_VkPipelineColorBlendStateCreateInfo("colour_blend_sci", readNode,
                                                          colour_blend_sci)) {
            newData.pColourBlendSCI = (VkPipelineColorBlendStateCreateInfo *)malloc(
                sizeof(VkPipelineColorBlendStateCreateInfo));
            *newData.pColourBlendSCI = colour_blend_sci;
        }
    } catch (foeYamlException const &e) {
        cleanup_foeMaterialCreateInfo(&newData);

        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_foeMaterialCreateInfo(std::string const &nodeName,
                                      foeMaterialCreateInfo const &data,
                                      YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // foeResourceID - fragmentShader
        if (data.fragmentShader != FOE_INVALID_ID) {
            yaml_write_foeResourceID("fragment_shader", data.fragmentShader, writeNode);
        }

        // foeResourceID - image
        if (data.image != FOE_INVALID_ID) {
            yaml_write_foeResourceID("image", data.image, writeNode);
        }

        // VkPipelineRasterizationStateCreateInfo* - pRasterizationSCI
        if (data.pRasterizationSCI) {
            yaml_write_VkPipelineRasterizationStateCreateInfo("rasterization_sci",
                                                              *data.pRasterizationSCI, writeNode);
        }

        // VkPipelineDepthStencilStateCreateInfo* - pDepthStencilSCI
        if (data.pDepthStencilSCI) {
            yaml_write_VkPipelineDepthStencilStateCreateInfo("depth_stencil_sci",
                                                             *data.pDepthStencilSCI, writeNode);
        }

        // VkPipelineColorBlendStateCreateInfo* - pColourBlendSCI
        if (data.pColourBlendSCI) {
            yaml_write_VkPipelineColorBlendStateCreateInfo("colour_blend_sci",
                                                           *data.pColourBlendSCI, writeNode);
        }
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

bool yaml_read_foeMeshFileCreateInfo(std::string const &nodeName,
                                     YAML::Node const &node,
                                     foeMeshFileCreateInfo &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    foeMeshFileCreateInfo newData = {};
    try {
        // char const * - pFile[null-terminated]
        if (std::string pFile; yaml_read_string("file", readNode, pFile)) {
            newData.pFile = (char *)malloc(pFile.size() + 1);
            memcpy((char *)newData.pFile, pFile.c_str(), pFile.size() + 1);
        }

        // char const * - pMesh[null-terminated]
        if (std::string pMesh; yaml_read_string("mesh", readNode, pMesh)) {
            newData.pMesh = (char *)malloc(pMesh.size() + 1);
            memcpy((char *)newData.pMesh, pMesh.c_str(), pMesh.size() + 1);
        }

        // aiPostProcessSteps - postProcessFlags
        if (std::string ppFlags; yaml_read_string("post_process_flags", readNode, ppFlags)) {
            foe_model_assimp_parse(ppFlags, &newData.postProcessFlags);
        }
    } catch (foeYamlException const &e) {
        cleanup_foeMeshFileCreateInfo(&newData);

        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_foeMeshFileCreateInfo(std::string const &nodeName,
                                      foeMeshFileCreateInfo const &data,
                                      YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // char const * - pFile[null-terminated]
        if (data.pFile) {
            yaml_write_string("file", data.pFile, writeNode);
        }

        // char const * - pMesh[null-terminated]
        if (data.pMesh) {
            yaml_write_string("mesh", data.pMesh, writeNode);
        }

        // aiPostProcessSteps - postProcessFlags
        if (data.postProcessFlags != 0) {
            std::string ppFlags;
            foe_model_assimp_serialize(data.postProcessFlags, &ppFlags);
            yaml_write_string("post_process_flags", ppFlags, writeNode);
        }
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

bool yaml_read_foeMeshIcosphereCreateInfo(std::string const &nodeName,
                                          YAML::Node const &node,
                                          foeMeshIcosphereCreateInfo &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    foeMeshIcosphereCreateInfo newData = {};
    try {
        // int - recursion
        yaml_read_int("recursion", readNode, newData.recursion);
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_foeMeshIcosphereCreateInfo(std::string const &nodeName,
                                           foeMeshIcosphereCreateInfo const &data,
                                           YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // int - recursion
        if (data.recursion != 0) {
            yaml_write_int("recursion", data.recursion, writeNode);
        }
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

bool yaml_read_foeShaderCreateInfo(std::string const &nodeName,
                                   YAML::Node const &node,
                                   foeShaderCreateInfo &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    foeShaderCreateInfo newData = {};
    try {
        // char const * - pFile[null-terminated]
        if (std::string pFile; yaml_read_string("file", readNode, pFile)) {
            newData.pFile = (char *)malloc(pFile.size() + 1);
            memcpy((char *)newData.pFile, pFile.c_str(), pFile.size() + 1);
        }

        // foeGfxVkShaderCreateInfo - gfxCreateInfo
        yaml_read_foeGfxVkShaderCreateInfo("gfx_create_info", readNode, newData.gfxCreateInfo);
    } catch (foeYamlException const &e) {
        cleanup_foeShaderCreateInfo(&newData);

        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_foeShaderCreateInfo(std::string const &nodeName,
                                    foeShaderCreateInfo const &data,
                                    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // char const * - pFile[null-terminated]
        if (data.pFile) {
            yaml_write_string("file", data.pFile, writeNode);
        }

        // foeGfxVkShaderCreateInfo - gfxCreateInfo
        if (foeGfxVkShaderCreateInfo tmp = {};
            !compare_foeGfxVkShaderCreateInfo(&data.gfxCreateInfo, &tmp)) {
            yaml_write_foeGfxVkShaderCreateInfo("gfx_create_info", data.gfxCreateInfo, writeNode);
        }
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

bool yaml_read_foeVertexDescriptorCreateInfo(std::string const &nodeName,
                                             YAML::Node const &node,
                                             foeEcsGroupTranslator groupTranslator,
                                             foeVertexDescriptorCreateInfo &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    foeVertexDescriptorCreateInfo newData = {};
    try {
        // foeResourceID - vertexShader
        yaml_read_foeResourceID("vertex_shader", readNode, groupTranslator, newData.vertexShader);

        // foeResourceID - tessellationControlShader
        yaml_read_foeResourceID("tessellation_control_shader", readNode, groupTranslator,
                                newData.tessellationControlShader);

        // foeResourceID - tessellationEvaluationShader
        yaml_read_foeResourceID("tessellation_evaluation_shader", readNode, groupTranslator,
                                newData.tessellationEvaluationShader);

        // foeResourceID - geometryShader
        yaml_read_foeResourceID("geometry_shader", readNode, groupTranslator,
                                newData.geometryShader);

        // VkPipelineVertexInputStateCreateInfo - vertexInputSCI
        yaml_read_VkPipelineVertexInputStateCreateInfo("vertex_input_sci", readNode,
                                                       newData.vertexInputSCI);

        // VkVertexInputBindingDescription* - pInputBindings[inputBindingCount]
        if (YAML::Node input_bindings_node = readNode["input_bindings"]; input_bindings_node) {
            // Set the associated control member
            newData.inputBindingCount = input_bindings_node.size();

            if (newData.inputBindingCount > 0) {
                newData.pInputBindings = (VkVertexInputBindingDescription *)malloc(
                    newData.inputBindingCount * sizeof(VkVertexInputBindingDescription));
                for (size_t i = 0; i < newData.inputBindingCount; ++i) {
                    YAML::Node subReadNode = input_bindings_node[i];
                    if (!yaml_read_VkVertexInputBindingDescription("", subReadNode,
                                                                   newData.pInputBindings[i])) {
                        throw foeYamlException{"input_bindings - Failed to read list-node"};
                    }
                }
            }
        }

        // VkVertexInputAttributeDescription* - pInputAttributes[inputAttributeCount]
        if (YAML::Node input_attributes_node = readNode["input_attributes"];
            input_attributes_node) {
            // Set the associated control member
            newData.inputAttributeCount = input_attributes_node.size();

            if (newData.inputAttributeCount > 0) {
                newData.pInputAttributes = (VkVertexInputAttributeDescription *)malloc(
                    newData.inputAttributeCount * sizeof(VkVertexInputAttributeDescription));
                for (size_t i = 0; i < newData.inputAttributeCount; ++i) {
                    YAML::Node subReadNode = input_attributes_node[i];
                    if (!yaml_read_VkVertexInputAttributeDescription("", subReadNode,
                                                                     newData.pInputAttributes[i])) {
                        throw foeYamlException{"input_attributes - Failed to read list-node"};
                    }
                }
            }
        }

        // VkPipelineInputAssemblyStateCreateInfo - inputAssemblySCI
        yaml_read_VkPipelineInputAssemblyStateCreateInfo("input_assembly_sci", readNode,
                                                         newData.inputAssemblySCI);

        // VkPipelineTessellationStateCreateInfo - tessellationSCI
        yaml_read_VkPipelineTessellationStateCreateInfo("tessellation_sci", readNode,
                                                        newData.tessellationSCI);
    } catch (foeYamlException const &e) {
        cleanup_foeVertexDescriptorCreateInfo(&newData);

        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_foeVertexDescriptorCreateInfo(std::string const &nodeName,
                                              foeVertexDescriptorCreateInfo const &data,
                                              YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // foeResourceID - vertexShader
        if (data.vertexShader != FOE_INVALID_ID) {
            yaml_write_foeResourceID("vertex_shader", data.vertexShader, writeNode);
        }

        // foeResourceID - tessellationControlShader
        if (data.tessellationControlShader != FOE_INVALID_ID) {
            yaml_write_foeResourceID("tessellation_control_shader", data.tessellationControlShader,
                                     writeNode);
        }

        // foeResourceID - tessellationEvaluationShader
        if (data.tessellationEvaluationShader != FOE_INVALID_ID) {
            yaml_write_foeResourceID("tessellation_evaluation_shader",
                                     data.tessellationEvaluationShader, writeNode);
        }

        // foeResourceID - geometryShader
        if (data.geometryShader != FOE_INVALID_ID) {
            yaml_write_foeResourceID("geometry_shader", data.geometryShader, writeNode);
        }

        // VkPipelineVertexInputStateCreateInfo - vertexInputSCI
        if (VkPipelineVertexInputStateCreateInfo tmp = {};
            !compare_VkPipelineVertexInputStateCreateInfo(&data.vertexInputSCI, &tmp)) {
            yaml_write_VkPipelineVertexInputStateCreateInfo("vertex_input_sci", data.vertexInputSCI,
                                                            writeNode);
        }

        // VkVertexInputBindingDescription* - pInputBindings[inputBindingCount]
        if (data.inputBindingCount > 0) {
            YAML::Node subWriteNode;

            for (size_t i = 0; i < data.inputBindingCount; ++i) {
                YAML::Node listNode;
                yaml_write_VkVertexInputBindingDescription("", data.pInputBindings[i], listNode);
                subWriteNode.push_back(listNode);
            }

            writeNode["input_bindings"] = subWriteNode;
        }

        // VkVertexInputAttributeDescription* - pInputAttributes[inputAttributeCount]
        if (data.inputAttributeCount > 0) {
            YAML::Node subWriteNode;

            for (size_t i = 0; i < data.inputAttributeCount; ++i) {
                YAML::Node listNode;
                yaml_write_VkVertexInputAttributeDescription("", data.pInputAttributes[i],
                                                             listNode);
                subWriteNode.push_back(listNode);
            }

            writeNode["input_attributes"] = subWriteNode;
        }

        // VkPipelineInputAssemblyStateCreateInfo - inputAssemblySCI
        if (VkPipelineInputAssemblyStateCreateInfo tmp = {};
            !compare_VkPipelineInputAssemblyStateCreateInfo(&data.inputAssemblySCI, &tmp)) {
            yaml_write_VkPipelineInputAssemblyStateCreateInfo("input_assembly_sci",
                                                              data.inputAssemblySCI, writeNode);
        }

        // VkPipelineTessellationStateCreateInfo - tessellationSCI
        if (VkPipelineTessellationStateCreateInfo tmp = {};
            !compare_VkPipelineTessellationStateCreateInfo(&data.tessellationSCI, &tmp)) {
            yaml_write_VkPipelineTessellationStateCreateInfo("tessellation_sci",
                                                             data.tessellationSCI, writeNode);
        }
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}
