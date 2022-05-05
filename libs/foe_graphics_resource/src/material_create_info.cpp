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

#include <foe/graphics/resource/material_create_info.hpp>

#include <vk_struct_cleanup.h>

foeMaterialCreateInfo::~foeMaterialCreateInfo() {
    if (hasColourBlendSCI)
        cleanup_VkPipelineColorBlendStateCreateInfo(&colourBlendSCI);
    if (hasDepthStencilSCI)
        cleanup_VkPipelineDepthStencilStateCreateInfo(&depthStencilSCI);
    if (hasRasterizationSCI)
        cleanup_VkPipelineRasterizationStateCreateInfo(&rasterizationSCI);
}

void foeDestroyMaterialCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo) {
    auto *pCI = (foeMaterialCreateInfo *)pCreateInfo;
    pCI->~foeMaterialCreateInfo();
}