// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "material.h"

#include <foe/graphics/resource/binary.h>
#include <foe/graphics/resource/cleanup.h>
#include <foe/graphics/resource/material.hpp>
#include <foe/graphics/resource/material_create_info.h>
#include <foe/graphics/resource/type_defs.h>
#include <foe/simulation/simulation.hpp>

#include "result.h"

extern "C" foeResultSet import_foeMaterialCreateInfo(void const *pReadBuffer,
                                                     uint32_t *pReadSize,
                                                     foeEcsGroupTranslator groupTranslator,
                                                     foeResourceCreateInfo *pResourceCI) {
    foeMaterialCreateInfo ciData = {};
    foeResourceCreateInfo createInfo;

    foeResultSet result =
        binary_read_foeMaterialCreateInfo(pReadBuffer, pReadSize, groupTranslator, &ciData);
    if (result.value != FOE_SUCCESS)
        return result;

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeMaterialCreateInfo *)pSrc;
        new (pDst) foeMaterialCreateInfo(std::move(*pSrcData));
    };

    result =
        foeCreateResourceCreateInfo(FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_CREATE_INFO,
                                    (PFN_foeResourceCreateInfoCleanup)cleanup_foeMaterialCreateInfo,
                                    sizeof(foeMaterialCreateInfo), &ciData, dataFn, &createInfo);

    if (result.value == FOE_SUCCESS)
        *pResourceCI = createInfo;
    return result;
}

extern "C" foeResultSet create_foeMaterialCreateInfo(foeResourceID resourceID,
                                                     foeResourceCreateInfo createInfo,
                                                     foeSimulation const *pSimulation) {
    foeResource material =
        foeResourcePoolAdd(pSimulation->resourcePool, resourceID,
                           FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL, sizeof(foeMaterial));

    if (material == FOE_NULL_HANDLE)
        return to_foeResult(FOE_GRAPHICS_RESOURCE_BINARY_ERROR_MATERIAL_RESOURCE_ALREADY_EXISTS);

    foeResourceDecrementRefCount(material);
    return to_foeResult(FOE_GRAPHICS_RESOURCE_BINARY_SUCCESS);
}