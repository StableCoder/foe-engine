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

#ifndef FOE_ECS_INDEX_GENERATOR_HPP
#define FOE_ECS_INDEX_GENERATOR_HPP

#include <foe/ecs/export.h>
#include <foe/ecs/id.h>
#include <foe/error_code.h>

#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <string_view>
#include <vector>

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
 * A fresh generator starts under the assumption that no indices exist. To change this, the
 * importState function can be called, or to otherwise get the current state exportState can be
 * called.
 *
 * To determine the totality of all IDs that are currently 'active', it will be all IDs from
 * foeIdIndexMinValue to the nextFreeID, minus any recycled IDs. This information can be retrieved
 * using the exportState function.
 *
 * @note This class is thread-safe.
 */
class foeIdIndexGenerator {
  public:
    /** Constructor.
     * @param groupId The GroupID of the indices we're generating for.
     */
    FOE_ECS_EXPORT foeIdIndexGenerator(foeIdGroup groupId);

    /** Returns either a recycled ID or a brand-new ID for use.
     * @return An unused ID. If no indexes are available, returns FOE_INVALID_ID instead.
     */
    FOE_ECS_EXPORT foeId generate();

    /** Frees an IndexID so that it may be recycled.
     * @param id The ID to recycle.
     * @return True if freed, false otherwise.
     *
     * EntityID must be from the same GroupID.
     */
    FOE_ECS_EXPORT bool free(foeId id);

    /** Frees an IndexID so that it may be recycled.
     * @param idList Set of IDs to be freed.
     * @return True if freed/recycled, false otherwise.
     *
     * IDs must be from the same GroupID, otherwise it will fail without recycling any of the IDs.
     */
    FOE_ECS_EXPORT bool free(uint32_t count, foeId *pEntities);

    /** Returns the objects associated GroupID
     * @return The class's group id.
     */
    FOE_ECS_EXPORT foeIdGroup groupID() const noexcept;

    /** Peeks at the next *fresh* index that will be generated.
     * @return The next fresh index that will be generated.
     * @warning Do NOT use this for new IDs. Use the generate() function for that. This is for
     * informational/debugging purposes only.
     *
     * This does not look at the next index, as it may be a recycled index instead, but
     * specifically at the next *fresh* index that hasn't yet been used at all.
     */
    FOE_ECS_EXPORT foeIdIndex peekNextFreshIndex() const noexcept;

    /** Returns the number of currently recyclable IDs available.
     * @return The current number of recyclable ids.
     */
    FOE_ECS_EXPORT std::size_t recyclable() const noexcept;

    FOE_ECS_EXPORT void forEachID(std::function<void(foeId)> callFn);

    /**
     * @brief Overwrites the current state of the generator with that provided
     * @param nextIndex is the next free index that would be generated
     * @param recycledCount is the integer related to the number of recycled Index IDs in pRecycled
     * @param pRecycledIndexes is a pointer to recycled Index IDs to be imported
     * @warning Overwrites the current state of the object
     */
    FOE_ECS_EXPORT
    foeResult importState(foeIdIndex nextIndex,
                          uint32_t recycledCount,
                          foeIdIndex const *pRecycledIndexes);

    /**
     * @brief Returns the current state of the object for import/export
     * @param[out] pNextIndex is a pointer representing the next fresh IndexID that would be
     * generated
     * @param[out] pRecycledCount is a pointer to an integer related to the number of recycled
     * IndexIDs available or the size of pRecycled, as described below.
     * @param[out] pRecycledIndexes is either NULL or a pointer to an array of foeIdIndex's
     * @return An appropriate error code, as described below.
     *
     * If either pNextIndex or pRecycled is NULL, then the number of recycled items available will
     * be returned via pRecycledCount. Otherwise pRecycledCount must point to a variable set by the
     * user to the size of the pRecycled array, and on the return the variable is overwritted with
     * the number of recycled IndexIDs actually writted to pRecycled.
     *
     * If pRecycledCount is less than the total size required to return all names, at most
     * pRecycledCount is written, and FOE_ECS_ERROR_INCOMPLETE will be returned instead of
     * FOE_ECS_SUCCESS, to indicate that not all IndexIDs were returned.
     */
    FOE_ECS_EXPORT foeResult exportState(foeIdIndex *pNextIndex,
                                         uint32_t *pRecycledCount,
                                         foeIdIndex *pRecycledIndexes);

  private:
    /// The group of the entity IDs being managed
    foeIdGroup const cGroupID;

    /// Synchronizes the recycle list
    std::mutex mSync;
    /// The next free IndexID, never yet used.
    std::atomic<foeIdIndexValue> mNextFreeID;
    /// The list of recyclable IndexIDs.
    std::queue<foeIdIndex> mRecycled;
    /// The number of currently recyclable IDs.
    size_t mNumRecycled;
};

#endif // FOE_ECS_INDEX_GENERATOR_HPP