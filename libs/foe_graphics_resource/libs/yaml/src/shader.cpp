// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "shader.hpp"

#include <foe/graphics/resource/cleanup.h>
#include <foe/graphics/resource/shader_create_info.h>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/resource/yaml/structs.hpp>
#include <foe/yaml/exception.hpp>

char const *yaml_shader_key() { return "shader_v1"; }

void yaml_read_shader(YAML::Node const &node,
                      foeEcsGroupTranslator groupTranslator,
                      foeResourceCreateInfo *pCreateInfo) {
    foeShaderCreateInfo shaderCI;
    foeResourceCreateInfo createInfo;

    yaml_read_foeShaderCreateInfo(yaml_shader_key(), node, shaderCI);

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeShaderCreateInfo *)pSrc;
        new (pDst) foeShaderCreateInfo(std::move(*pSrcData));
    };

    foeResultSet result =
        foeCreateResourceCreateInfo(FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_CREATE_INFO,
                                    (PFN_foeResourceCreateInfoCleanup)cleanup_foeShaderCreateInfo,
                                    sizeof(foeShaderCreateInfo), &shaderCI, dataFn, &createInfo);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        throw foeYamlException{std::string{"Failed to create foeShaderCreateInfo due to error: "} +
                               buffer};
    }

    *pCreateInfo = createInfo;
}

auto yaml_write_shader(foeShaderCreateInfo const &data) -> YAML::Node {
    YAML::Node definition;

    yaml_write_foeShaderCreateInfo("", data, definition);

    return definition;
}