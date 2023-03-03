// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "material.h"

#include <foe/graphics/resource/binary.h>
#include <foe/graphics/resource/cleanup.h>
#include <foe/graphics/resource/material_create_info.h>
#include <foe/graphics/resource/type_defs.h>

#include "result.h"

#include <new>
#include <utility>

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