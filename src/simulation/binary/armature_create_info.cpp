// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "armature_create_info.h"

#include "../armature.hpp"
#include "../armature_create_info.h"
#include "../binary.h"
#include "../cleanup.h"
#include "../type_defs.h"
#include "result.h"

extern "C" foeResultSet import_foeArmatureCreateInfo(void const *pReadBuffer,
                                                     uint32_t *pReadSize,
                                                     foeEcsGroupTranslator groupTranslator,
                                                     foeResourceCreateInfo *pResourceCI) {
    foeArmatureCreateInfo ciData = {};
    foeResourceCreateInfo createInfo;

    foeResultSet result = binary_read_foeArmatureCreateInfo(pReadBuffer, pReadSize, &ciData);
    if (result.value != FOE_SUCCESS)
        return result;

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeArmatureCreateInfo *)pSrc;
        new (pDst) foeArmatureCreateInfo(std::move(*pSrcData));
    };

    result =
        foeCreateResourceCreateInfo(FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_CREATE_INFO,
                                    (PFN_foeResourceCreateInfoCleanup)cleanup_foeArmatureCreateInfo,
                                    sizeof(foeArmatureCreateInfo), &ciData, dataFn, &createInfo);

    if (result.value == FOE_SUCCESS)
        *pResourceCI = createInfo;
    return result;
}