/*
    Copyright (C) 2021-2022 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

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

FOE_ECS_EXPORT foeResult foeEcsCreateIndexes(foeIdGroup groupID, foeEcsIndexes *pIndexes);

FOE_ECS_EXPORT void foeEcsDestroyIndexes(foeEcsIndexes indexes);

FOE_ECS_EXPORT foeIdGroup foeEcsIndexesGetGroupID(foeEcsIndexes indexes);

FOE_ECS_EXPORT foeResult foeEcsGenerateID(foeEcsIndexes indexes, foeId *pID);

FOE_ECS_EXPORT foeResult foeEcsFreeID(foeEcsIndexes indexes, foeId id);

FOE_ECS_EXPORT foeResult foeEcsFreeIDs(foeEcsIndexes indexes, uint32_t idCount, foeId const *pIDs);

FOE_ECS_EXPORT void foeEcsForEachID(foeEcsIndexes indexes,
                                    PFN_foeEcsForEachCall forEachCall,
                                    void *pCallContext);

FOE_ECS_EXPORT foeResult foeEcsImportIndexes(foeEcsIndexes indexes,
                                             foeIdIndex nextNewIndex,
                                             uint32_t recycledCount,
                                             foeIdIndex const *pRecycledIndexes);

FOE_ECS_EXPORT foeResult foeEcsExportIndexes(foeEcsIndexes indexes,
                                             foeIdIndex *pNextNewIndex,
                                             uint32_t *pRecycledCount,
                                             foeIdIndex *pRecycledIndexes);

#ifdef __cplusplus
}
#endif

#endif // FOE_ECS_INDEXES_H