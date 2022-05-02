/*
    Copyright (C) 2022 George Cave.

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

#ifndef FOE_GRAPHICS_RESOURCE_MATERIAL_CREATE_INFO_HPP
#define FOE_GRAPHICS_RESOURCE_MATERIAL_CREATE_INFO_HPP

#include <foe/ecs/id.h>
#include <foe/graphics/resource/export.h>
#include <foe/resource/create_info.h>
#include <vulkan/vulkan.h>

#include <vector>

struct FOE_GFX_RES_EXPORT foeMaterialCreateInfo {
    ~foeMaterialCreateInfo();

    foeId fragmentShader = FOE_INVALID_ID;
    foeId image = FOE_INVALID_ID;
    bool hasRasterizationSCI;
    VkPipelineRasterizationStateCreateInfo rasterizationSCI;
    bool hasDepthStencilSCI;
    VkPipelineDepthStencilStateCreateInfo depthStencilSCI;
    bool hasColourBlendSCI;
    VkPipelineColorBlendStateCreateInfo colourBlendSCI;
    std::vector<VkPipelineColorBlendAttachmentState> colourBlendAttachments;
};

FOE_GFX_RES_EXPORT void foeDestroyMaterialCreateInfo(foeResourceCreateInfoType type,
                                                     void *pCreateInfo);

#endif // FOE_GRAPHICS_RESOURCE_MATERIAL_CREATE_INFO_HPP