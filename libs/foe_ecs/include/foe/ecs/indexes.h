// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_ECS_INDEXES_H
#define FOE_ECS_INDEXES_H

#include <foe/ecs/export.h>
#include <foe/ecs/id.h>
#include <foe/error_code.h>
#include <foe/handle.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*PFN_foeEcsForEachCall)(void *, foeId);

/** @brief Used to generate/free/manage indices within a group
 *
 * For each group, a particular index should be unique, representing a single specific object in the
 * simulation.
 *
 * At it's heart, to determine unique IDs it looks at two items, the first is the set of recycled
 * indices which is held in a queue, access to which is guarded by a mutex. If there are no items
 * that have been freed are available, it will move on to using the nextFreeID, which is a value
 * that has never been entered into circulation before.
 *
 * A fresh indexes starts under the assumption that no indices exist. To change this, the
 * importState function can be called, or to otherwise get the current state exportState can be
 * called.
 *
 * To determine the totality of all IDs that are currently 'active', it will be all IDs from
 * foeIdIndexMinValue to the nextFreeID, minus any recycled IDs. This information can be retrieved
 * using the exportState function.
 *
 * @note This class is thread-safe.
 */
FOE_DEFINE_HANDLE(foeEcsIndexes)

FOE_ECS_EXPORT foeResultSet foeEcsCreateIndexes(foeIdGroup groupID, foeEcsIndexes *pIndexes);

FOE_ECS_EXPORT void foeEcsDestroyIndexes(foeEcsIndexes indexes);

FOE_ECS_EXPORT foeIdGroup foeEcsIndexesGetGroupID(foeEcsIndexes indexes);

FOE_ECS_EXPORT foeResultSet foeEcsGenerateID(foeEcsIndexes indexes, foeId *pID);

FOE_ECS_EXPORT foeResultSet foeEcsFreeID(foeEcsIndexes indexes, foeId id);

FOE_ECS_EXPORT foeResultSet foeEcsFreeIDs(foeEcsIndexes indexes,
                                          uint32_t idCount,
                                          foeId const *pIDs);

FOE_ECS_EXPORT void foeEcsForEachID(foeEcsIndexes indexes,
                                    PFN_foeEcsForEachCall forEachCall,
                                    void *pCallContext);

FOE_ECS_EXPORT foeResultSet foeEcsImportIndexes(foeEcsIndexes indexes,
                                                foeIdIndex nextNewIndex,
                                                uint32_t recycledCount,
                                                foeIdIndex const *pRecycledIndexes);

FOE_ECS_EXPORT foeResultSet foeEcsExportIndexes(foeEcsIndexes indexes,
                                                foeIdIndex *pNextNewIndex,
                                                uint32_t *pRecycledCount,
                                                foeIdIndex *pRecycledIndexes);

#ifdef __cplusplus
}
#endif

#endif // FOE_ECS_INDEXES_H