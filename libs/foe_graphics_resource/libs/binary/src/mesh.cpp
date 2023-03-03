// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "mesh.h"

#include <foe/graphics/resource/binary.h>
#include <foe/graphics/resource/cleanup.h>
#include <foe/graphics/resource/mesh_create_info.h>
#include <foe/graphics/resource/type_defs.h>

#include "result.h"

#include <new>
#include <utility>

extern "C" foeResultSet import_foeMeshFileCreateInfo(void const *pReadBuffer,
                                                     uint32_t *pReadSize,
                                                     foeEcsGroupTranslator groupTranslator,
                                                     foeResourceCreateInfo *pResourceCI) {
    foeMeshFileCreateInfo ciData = {};
    foeResourceCreateInfo createInfo;

    foeResultSet result = binary_read_foeMeshFileCreateInfo(pReadBuffer, pReadSize, &ciData);
    if (result.value != FOE_SUCCESS)
        return result;

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeMeshFileCreateInfo *)pSrc;
        new (pDst) foeMeshFileCreateInfo(std::move(*pSrcData));
    };

    result =
        foeCreateResourceCreateInfo(FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_FILE_CREATE_INFO,
                                    (PFN_foeResourceCreateInfoCleanup)cleanup_foeMeshFileCreateInfo,
                                    sizeof(foeMeshFileCreateInfo), &ciData, dataFn, &createInfo);

    if (result.value == FOE_SUCCESS)
        *pResourceCI = createInfo;
    return result;
}

extern "C" foeResultSet import_foeMeshCubeCreateInfo(void const *pReadBuffer,
                                                     uint32_t *pReadSize,
                                                     foeEcsGroupTranslator groupTranslator,
                                                     foeResourceCreateInfo *pResourceCI) {
    foeMeshCubeCreateInfo ciData = {};
    foeResourceCreateInfo createInfo;

    foeResultSet result = binary_read_foeMeshCubeCreateInfo(pReadBuffer, pReadSize, &ciData);
    if (result.value != FOE_SUCCESS)
        return result;

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeMeshCubeCreateInfo *)pSrc;
        new (pDst) foeMeshCubeCreateInfo(std::move(*pSrcData));
    };

    result = foeCreateResourceCreateInfo(FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_CUBE_CREATE_INFO,
                                         nullptr, sizeof(foeMeshCubeCreateInfo), &ciData, dataFn,
                                         &createInfo);

    if (result.value == FOE_SUCCESS)
        *pResourceCI = createInfo;
    return result;
}

extern "C" foeResultSet import_foeMeshIcosphereCreateInfo(void const *pReadBuffer,
                                                          uint32_t *pReadSize,
                                                          foeEcsGroupTranslator groupTranslator,
                                                          foeResourceCreateInfo *pResourceCI) {
    foeMeshIcosphereCreateInfo ciData = {};
    foeResourceCreateInfo createInfo;

    foeResultSet result = binary_read_foeMeshIcosphereCreateInfo(pReadBuffer, pReadSize, &ciData);
    if (result.value != FOE_SUCCESS)
        return result;

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeMeshIcosphereCreateInfo *)pSrc;
        new (pDst) foeMeshIcosphereCreateInfo(std::move(*pSrcData));
    };

    result = foeCreateResourceCreateInfo(
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_ICOSPHERE_CREATE_INFO, nullptr,
        sizeof(foeMeshIcosphereCreateInfo), &ciData, dataFn, &createInfo);

    if (result.value == FOE_SUCCESS)
        *pResourceCI = createInfo;
    return result;
}
