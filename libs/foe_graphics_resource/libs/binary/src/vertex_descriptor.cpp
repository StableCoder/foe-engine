// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "vertex_descriptor.h"

#include <foe/graphics/resource/binary.h>
#include <foe/graphics/resource/cleanup.h>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/resource/vertex_descriptor.hpp>
#include <foe/graphics/resource/vertex_descriptor_create_info.h>
#include <foe/simulation/simulation.hpp>

#include "result.h"

extern "C" foeResultSet import_foeVertexDescriptorCreateInfo(void const *pReadBuffer,
                                                             uint32_t *pReadSize,
                                                             foeEcsGroupTranslator groupTranslator,
                                                             foeResourceCreateInfo *pResourceCI) {
    foeVertexDescriptorCreateInfo ciData = {};
    foeResourceCreateInfo createInfo;

    foeResultSet result =
        binary_read_foeVertexDescriptorCreateInfo(pReadBuffer, pReadSize, groupTranslator, &ciData);
    if (result.value != FOE_SUCCESS)
        return result;

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeVertexDescriptorCreateInfo *)pSrc;
        new (pDst) foeVertexDescriptorCreateInfo(std::move(*pSrcData));
    };

    result = foeCreateResourceCreateInfo(
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_CREATE_INFO,
        (PFN_foeResourceCreateInfoCleanup)cleanup_foeVertexDescriptorCreateInfo,
        sizeof(foeVertexDescriptorCreateInfo), &ciData, dataFn, &createInfo);

    if (result.value == FOE_SUCCESS)
        *pResourceCI = createInfo;
    return result;
}

extern "C" foeResultSet create_foeVertexDescriptorCreateInfo(foeResourceID resourceID,
                                                             foeResourceCreateInfo createInfo,
                                                             foeSimulation const *pSimulation) {
    foeResource vertex_descriptor = foeResourcePoolAdd(
        pSimulation->resourcePool, resourceID,
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR, sizeof(foeVertexDescriptor));

    if (vertex_descriptor == FOE_NULL_HANDLE)
        return to_foeResult(
            FOE_GRAPHICS_RESOURCE_BINARY_ERROR_VERTEX_DESCRIPTOR_RESOURCE_ALREADY_EXISTS);

    foeResourceDecrementRefCount(vertex_descriptor);
    return to_foeResult(FOE_GRAPHICS_RESOURCE_BINARY_SUCCESS);
}