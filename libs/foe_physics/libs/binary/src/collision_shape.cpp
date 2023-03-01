// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "collision_shape.h"

#include <foe/physics/binary.h>
#include <foe/physics/resource/collision_shape.hpp>
#include <foe/physics/resource/collision_shape_create_info.hpp>
#include <foe/physics/type_defs.h>
#include <foe/simulation/simulation.hpp>

#include "result.h"

extern "C" foeResultSet import_foeCollisionShapeCreateInfo(void const *pReadBuffer,
                                                           uint32_t *pReadSize,
                                                           foeEcsGroupTranslator groupTranslator,
                                                           foeResourceCreateInfo *pResourceCI) {
    foeCollisionShapeCreateInfo ciData = {};
    foeResourceCreateInfo createInfo;

    foeResultSet result = binary_read_foeCollisionShapeCreateInfo(pReadBuffer, pReadSize, &ciData);
    if (result.value != FOE_SUCCESS)
        return result;

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeCollisionShapeCreateInfo *)pSrc;
        new (pDst) foeCollisionShapeCreateInfo(std::move(*pSrcData));
    };

    result = foeCreateResourceCreateInfo(FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_CREATE_INFO,
                                         nullptr, sizeof(foeCollisionShapeCreateInfo), &ciData,
                                         dataFn, &createInfo);

    if (result.value == FOE_SUCCESS)
        *pResourceCI = createInfo;
    return result;
}

extern "C" foeResultSet create_foeCollisionShapeCreateInfo(foeResourceID resource,
                                                           foeResourceCreateInfo resourceCI,
                                                           foeSimulation const *pSimulation) {
    foeResource collisionShape =
        foeResourcePoolAdd(pSimulation->resourcePool, resource,
                           FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE, sizeof(foeCollisionShape));

    if (collisionShape == FOE_NULL_HANDLE)
        return to_foeResult(FOE_PHYSICS_BINARY_ERROR_COLLISION_SHAPE_ALREADY_EXISTS);

    foeResourceDecrementRefCount(collisionShape);
    return to_foeResult(FOE_PHYSICS_BINARY_SUCCESS);
}