// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "shader.h"

#include <foe/graphics/resource/binary.h>
#include <foe/graphics/resource/cleanup.h>
#include <foe/graphics/resource/shader.hpp>
#include <foe/graphics/resource/shader_create_info.h>
#include <foe/graphics/resource/type_defs.h>
#include <foe/simulation/simulation.hpp>

#include "result.h"

extern "C" foeResultSet import_foeShaderCreateInfo(void const *pReadBuffer,
                                                   uint32_t *pReadSize,
                                                   foeEcsGroupTranslator groupTranslator,
                                                   foeResourceCreateInfo *pResourceCI) {
    foeShaderCreateInfo ciData = {};
    foeResourceCreateInfo createInfo;

    foeResultSet result = binary_read_foeShaderCreateInfo(pReadBuffer, pReadSize, &ciData);
    if (result.value != FOE_SUCCESS)
        return result;

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeShaderCreateInfo *)pSrc;
        new (pDst) foeShaderCreateInfo(std::move(*pSrcData));
    };

    result =
        foeCreateResourceCreateInfo(FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_CREATE_INFO,
                                    (PFN_foeResourceCreateInfoCleanup)cleanup_foeShaderCreateInfo,
                                    sizeof(foeShaderCreateInfo), &ciData, dataFn, &createInfo);

    if (result.value == FOE_SUCCESS)
        *pResourceCI = createInfo;
    return result;
}

extern "C" foeResultSet create_foeShaderCreateInfo(foeResourceID resourceID,
                                                   foeResourceCreateInfo createInfo,
                                                   foeSimulation const *pSimulation) {
    foeResource shader =
        foeResourcePoolAdd(pSimulation->resourcePool, resourceID,
                           FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER, sizeof(foeShader));

    if (shader == FOE_NULL_HANDLE)
        return to_foeResult(FOE_GRAPHICS_RESOURCE_BINARY_ERROR_SHADER_RESOURCE_ALREADY_EXISTS);

    foeResourceDecrementRefCount(shader);
    return to_foeResult(FOE_GRAPHICS_RESOURCE_BINARY_SUCCESS);
}