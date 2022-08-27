// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/compare.h>

#include <foe/graphics/resource/image_create_info.h>
#include <foe/graphics/resource/material_create_info.h>
#include <foe/graphics/resource/mesh_create_info.h>
#include <foe/graphics/resource/shader_create_info.h>
#include <foe/graphics/resource/vertex_descriptor_create_info.h>
#include <foe/graphics/vk/compare.h>
#include <vk_struct_compare.h>

#include <string.h>

bool compare_foeImageCreateInfo(foeImageCreateInfo const *pData1,
                                foeImageCreateInfo const *pData2) {
    // char const * - pFile[null-terminated]
    if (pData1->pFile != pData2->pFile || pData1->pFile == NULL || pData2->pFile == NULL ||
        strcmp(pData1->pFile, pData2->pFile) != 0) {
        return false;
    }

    return true;
}

bool compare_foeMeshFileCreateInfo(foeMeshFileCreateInfo const *pData1,
                                   foeMeshFileCreateInfo const *pData2) {
    // char const * - pFile[null-terminated]
    if (pData1->pFile != pData2->pFile || pData1->pFile == NULL || pData2->pFile == NULL ||
        strcmp(pData1->pFile, pData2->pFile) != 0) {
        return false;
    }

    // char const * - pMesh[null-terminated]
    if (pData1->pMesh != pData2->pMesh || pData1->pMesh == NULL || pData2->pMesh == NULL ||
        strcmp(pData1->pMesh, pData2->pMesh) != 0) {
        return false;
    }

    // unsigned int - postProcessFlags
    if (pData1->postProcessFlags != pData2->postProcessFlags) {
        return false;
    }

    return true;
}

bool compare_foeMeshCubeCreateInfo(foeMeshCubeCreateInfo const *pData1,
                                   foeMeshCubeCreateInfo const *pData2) {
    return true;
}

bool compare_foeMeshIcosphereCreateInfo(foeMeshIcosphereCreateInfo const *pData1,
                                        foeMeshIcosphereCreateInfo const *pData2) {
    // int - recursion
    if (pData1->recursion != pData2->recursion) {
        return false;
    }

    return true;
}

bool compare_foeMaterialCreateInfo(foeMaterialCreateInfo const *pData1,
                                   foeMaterialCreateInfo const *pData2) {
    // foeResourceID - fragmentShader
    if (pData1->fragmentShader != pData2->fragmentShader) {
        return false;
    }

    // foeResourceID - image
    if (pData1->image != pData2->image) {
        return false;
    }

    // VkPipelineRasterizationStateCreateInfo* - pRasterizationSCI
    if (pData1->pRasterizationSCI != pData2->pRasterizationSCI) {
        if (pData1->pRasterizationSCI == NULL || pData2->pRasterizationSCI == NULL) {
            return false;
        }

        if (!compare_VkPipelineRasterizationStateCreateInfo(pData1->pRasterizationSCI,
                                                            pData2->pRasterizationSCI)) {
            return false;
        }
    }

    // VkPipelineDepthStencilStateCreateInfo* - pDepthStencilSCI
    if (pData1->pDepthStencilSCI != pData2->pDepthStencilSCI) {
        if (pData1->pDepthStencilSCI == NULL || pData2->pDepthStencilSCI == NULL) {
            return false;
        }

        if (!compare_VkPipelineDepthStencilStateCreateInfo(pData1->pDepthStencilSCI,
                                                           pData2->pDepthStencilSCI)) {
            return false;
        }
    }

    // VkPipelineColorBlendStateCreateInfo* - pColourBlendSCI
    if (pData1->pColourBlendSCI != pData2->pColourBlendSCI) {
        if (pData1->pColourBlendSCI == NULL || pData2->pColourBlendSCI == NULL) {
            return false;
        }

        if (!compare_VkPipelineColorBlendStateCreateInfo(pData1->pColourBlendSCI,
                                                         pData2->pColourBlendSCI)) {
            return false;
        }
    }

    return true;
}

bool compare_foeShaderCreateInfo(foeShaderCreateInfo const *pData1,
                                 foeShaderCreateInfo const *pData2) {
    // char const * - pFile[null-terminated]
    if (pData1->pFile != pData2->pFile || pData1->pFile == NULL || pData2->pFile == NULL ||
        strcmp(pData1->pFile, pData2->pFile) != 0) {
        return false;
    }

    // foeGfxVkShaderCreateInfo - gfxCreateInfo
    if (!compare_foeGfxVkShaderCreateInfo(&pData1->gfxCreateInfo, &pData2->gfxCreateInfo)) {
        return false;
    }

    return true;
}

bool compare_foeVertexDescriptorCreateInfo(foeVertexDescriptorCreateInfo const *pData1,
                                           foeVertexDescriptorCreateInfo const *pData2) {
    // uint32_t - inputBindingCount
    if (pData1->inputBindingCount != pData2->inputBindingCount) {
        return false;
    }

    // uint32_t - inputAttributeCount
    if (pData1->inputAttributeCount != pData2->inputAttributeCount) {
        return false;
    }

    // foeResourceID - vertexShader
    if (pData1->vertexShader != pData2->vertexShader) {
        return false;
    }

    // foeResourceID - tessellationControlShader
    if (pData1->tessellationControlShader != pData2->tessellationControlShader) {
        return false;
    }

    // foeResourceID - tessellationEvaluationShader
    if (pData1->tessellationEvaluationShader != pData2->tessellationEvaluationShader) {
        return false;
    }

    // foeResourceID - geometryShader
    if (pData1->geometryShader != pData2->geometryShader) {
        return false;
    }

    // VkPipelineVertexInputStateCreateInfo - vertexInputSCI
    if (!compare_VkPipelineVertexInputStateCreateInfo(&pData1->vertexInputSCI,
                                                      &pData2->vertexInputSCI)) {
        return false;
    }

    // VkVertexInputBindingDescription* - pInputBindings[inputBindingCount]
    if (pData1->pInputBindings != pData2->pInputBindings) {
        if (pData1->pInputBindings == NULL || pData2->pInputBindings == NULL) {
            return false;
        }

        for (size_t i = 0; i < pData1->inputBindingCount; ++i) {
            if (!compare_VkVertexInputBindingDescription(pData1->pInputBindings + i,
                                                         pData2->pInputBindings + i)) {
                return false;
            }
        }
    }

    // VkVertexInputAttributeDescription* - pInputAttributes[inputAttributeCount]
    if (pData1->pInputAttributes != pData2->pInputAttributes) {
        if (pData1->pInputAttributes == NULL || pData2->pInputAttributes == NULL) {
            return false;
        }

        for (size_t i = 0; i < pData1->inputAttributeCount; ++i) {
            if (!compare_VkVertexInputAttributeDescription(pData1->pInputAttributes + i,
                                                           pData2->pInputAttributes + i)) {
                return false;
            }
        }
    }

    // VkPipelineInputAssemblyStateCreateInfo - inputAssemblySCI
    if (!compare_VkPipelineInputAssemblyStateCreateInfo(&pData1->inputAssemblySCI,
                                                        &pData2->inputAssemblySCI)) {
        return false;
    }

    // VkPipelineTessellationStateCreateInfo - tessellationSCI
    if (!compare_VkPipelineTessellationStateCreateInfo(&pData1->tessellationSCI,
                                                       &pData2->tessellationSCI)) {
        return false;
    }

    return true;
}
