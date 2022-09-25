// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "camera.h"

#include <foe/simulation/simulation.hpp>

#include "../binary.h"
#include "../camera.hpp"
#include "../camera_pool.hpp"
#include "result.h"

extern "C" foeResultSet export_foeCamera(foeEntityID entity,
                                         foeSimulation const *pSimulation,
                                         foeImexBinarySet *pBinarySet) {
    foeResultSet result = to_foeResult(FOE_BRINGUP_BINARY_DATA_NOT_EXPORTED);
    foeImexBinarySet set = {};

    auto *pComponentPool = (foeCameraPool *)foeSimulationGetComponentPool(
        pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL);
    if (pComponentPool) {
        if (auto searchIt = pComponentPool->find(entity); searchIt != pComponentPool->size()) {
            auto *pComponentData = pComponentPool->begin<1>() + searchIt;

            result = binary_write_foeCamera(pComponentData->get(), &set.dataSize, nullptr);
            if (result.value != FOE_SUCCESS)
                goto COMPONENT_EXPORT_FAILED;

            set.pData = malloc(set.dataSize);
            if (set.pData == NULL) {
                result = to_foeResult(FOE_BRINGUP_BINARY_ERROR_OUT_OF_MEMORY);
                goto COMPONENT_EXPORT_FAILED;
            }

            result = binary_write_foeCamera(pComponentData->get(), &set.dataSize, set.pData);
            set.pKey = binary_key_foeCamera();
        }
    }

COMPONENT_EXPORT_FAILED:
    if (result.value != FOE_SUCCESS && set.pData)
        free(set.pData);
    if (result.value == FOE_SUCCESS)
        *pBinarySet = set;

    return result;
}

extern "C" foeResultSet import_foeCamera(void const *pReadBuffer,
                                         uint32_t *pReadSize,
                                         foeEcsGroupTranslator groupTranslator,
                                         foeEntityID entity,
                                         foeSimulation const *pSimulation) {
    auto *pComponentPool = (foeCameraPool *)foeSimulationGetComponentPool(
        pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL);
    if (pComponentPool == nullptr)
        return to_foeResult(FOE_BRINGUP_BINARY_ERROR_CAMERA_POOL_NOT_FOUND);

    std::unique_ptr<foeCamera> pComponentData(new foeCamera);
    foeResultSet result = binary_read_foeCamera(pReadBuffer, pReadSize, pComponentData.get());
    if (result.value == FOE_SUCCESS)
        pComponentPool->insert(entity, std::move(pComponentData));

    return result;
}