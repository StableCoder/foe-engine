// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "vertex_descriptor.h"

#include <foe/graphics/resource/binary.h>
#include <foe/graphics/resource/cleanup.h>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/resource/vertex_descriptor_create_info.h>

#include "result.h"

#include <new>
#include <utility>

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