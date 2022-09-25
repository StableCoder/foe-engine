// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "image.h"

#include <foe/graphics/resource/binary.h>
#include <foe/graphics/resource/cleanup.h>
#include <foe/graphics/resource/image.hpp>
#include <foe/graphics/resource/image_create_info.h>
#include <foe/graphics/resource/type_defs.h>
#include <foe/simulation/simulation.hpp>

#include "result.h"

extern "C" foeResultSet import_foeImageCreateInfo(void const *pReadBuffer,
                                                  uint32_t *pReadSize,
                                                  foeEcsGroupTranslator groupTranslator,
                                                  foeResourceCreateInfo *pResourceCI) {
    foeImageCreateInfo ciData = {};
    foeResourceCreateInfo createInfo;

    foeResultSet result = binary_read_foeImageCreateInfo(pReadBuffer, pReadSize, &ciData);
    if (result.value != FOE_SUCCESS)
        return result;

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeImageCreateInfo *)pSrc;
        new (pDst) foeImageCreateInfo(std::move(*pSrcData));
    };

    result =
        foeCreateResourceCreateInfo(FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                    (PFN_foeResourceCreateInfoCleanup)cleanup_foeImageCreateInfo,
                                    sizeof(foeImageCreateInfo), &ciData, dataFn, &createInfo);

    if (result.value == FOE_SUCCESS)
        *pResourceCI = createInfo;
    return result;
}

extern "C" foeResultSet create_foeImageCreateInfo(foeResourceID resourceID,
                                                  foeResourceCreateInfo createInfo,
                                                  foeSimulation const *pSimulation) {
    foeResource image =
        foeResourcePoolAdd(pSimulation->resourcePool, resourceID,
                           FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE, sizeof(foeImage));

    if (image == FOE_NULL_HANDLE)
        return to_foeResult(FOE_GRAPHICS_RESOURCE_BINARY_ERROR_IMAGE_RESOURCE_ALREADY_EXISTS);

    return to_foeResult(FOE_GRAPHICS_RESOURCE_BINARY_SUCCESS);
}