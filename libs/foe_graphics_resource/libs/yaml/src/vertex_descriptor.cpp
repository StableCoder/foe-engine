// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "vertex_descriptor.hpp"

#include <foe/graphics/resource/cleanup.h>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/resource/vertex_descriptor_create_info.h>
#include <foe/graphics/resource/yaml/structs.hpp>
#include <foe/yaml/exception.hpp>

char const *yaml_vertex_descriptor_key() { return "vertex_descriptor_v1"; }

void yaml_read_vertex_descriptor(YAML::Node const &node,
                                 foeEcsGroupTranslator groupTranslator,
                                 foeResourceCreateInfo *pCreateInfo) {
    foeVertexDescriptorCreateInfo vdCI{};
    foeResourceCreateInfo createInfo;

    yaml_read_foeVertexDescriptorCreateInfo(yaml_vertex_descriptor_key(), node, groupTranslator,
                                            vdCI);

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeVertexDescriptorCreateInfo *)pSrc;
        new (pDst) foeVertexDescriptorCreateInfo(std::move(*pSrcData));
    };

    foeResultSet result = foeCreateResourceCreateInfo(
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_CREATE_INFO,
        (PFN_foeResourceCreateInfoCleanup)cleanup_foeVertexDescriptorCreateInfo,
        sizeof(foeVertexDescriptorCreateInfo), &vdCI, dataFn, &createInfo);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        throw foeYamlException{
            std::string{"Failed to create foeVertexDescriptorCreateInfo due to error: "} + buffer};
    }

    *pCreateInfo = createInfo;
}

auto yaml_write_vertex_descriptor(foeVertexDescriptorCreateInfo const &vertexDescriptor)
    -> YAML::Node {
    YAML::Node definition;

    yaml_write_foeVertexDescriptorCreateInfo("", vertexDescriptor, definition);

    return definition;
}