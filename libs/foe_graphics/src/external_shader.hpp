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

#ifndef EXTERNAL_SHADER_HPP
#define EXTERNAL_SHADER_HPP

#include <vk_error_code.hpp>
#include <vulkan/vulkan.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

#include "gfx_log.hpp"

auto loadShaderDataFromFile(std::filesystem::path const &shaderPath) -> std::vector<std::byte> {
    std::vector<std::byte> shaderData;

    // Open file
    std::ifstream file(shaderPath, std::ifstream::binary | std::ifstream::in | std::ifstream::ate);
    if (!file.is_open())
        return shaderData;

    auto size = file.tellg();
    if (size % 4 != 0) // Must be multiple of 4
        return shaderData;

    file.seekg(0);

    shaderData.resize(size);
    file.read(reinterpret_cast<char *>(shaderData.data()), size);

    return shaderData;
}

bool loadExternalShader(std::string const &externalShader,
                        VkDevice device,
                        VkShaderModule *pShaderModule) {
    // Shader Binary Data
    if (!std::filesystem::exists(externalShader)) {
        FOE_LOG(Graphics, Error, "Failed to find external shader file: {}", externalShader)
        return false;
    }

    auto shaderData = loadShaderDataFromFile(externalShader);
    if (shaderData.empty()) {
        FOE_LOG(Graphics, Error, "Failed to load shader module from file: {}", externalShader)
        return false;
    }

    VkShaderModuleCreateInfo shaderCI{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = shaderData.size(),
        .pCode = reinterpret_cast<uint32_t *>(shaderData.data()),
    };

    std::error_code errC = vkCreateShaderModule(device, &shaderCI, nullptr, pShaderModule);
    if (errC) {
        FOE_LOG(Graphics, Error, "Failed to create VkShaderMovule loaded from '{}' with error: {}",
                externalShader, errC.message())
        return false;
    }

    return true;
}

#endif // EXTERNAL_SHADER_HPP