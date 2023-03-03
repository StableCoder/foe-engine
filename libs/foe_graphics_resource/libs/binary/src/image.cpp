// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "image.h"

#include <foe/graphics/resource/binary.h>
#include <foe/graphics/resource/cleanup.h>
#include <foe/graphics/resource/image_create_info.h>
#include <foe/graphics/resource/type_defs.h>

#include "result.h"

#include <new>
#include <utility>

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