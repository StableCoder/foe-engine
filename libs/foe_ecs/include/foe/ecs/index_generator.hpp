/*
    Copyright (C) 2021 George Cave.

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
#include <foe/ecs/id.hpp>

#include <mutex>
#include <queue>
#include <string>
#include <string_view>
#include <vector>

class foeEcsIndexGenerator {
  public:
    /** Constructor.
     * @param name Name of the group, should be unique, or it won't be accepted by foe::ecs::Groups.
     * @param baseID The BaseID of the indices we're generating for.
     */
    FOE_ECS_EXPORT foeEcsIndexGenerator(std::string_view name, foeIdGroup groupId);

    /** Generates an unused IndexID for use in an EntityID.
     * @return An unused EntityID. If no indexes are available, returns eInvalidID instead.
     */
    FOE_ECS_EXPORT foeId generate();

    /** Generates an unused foeId that is set to represent a resource object
     * @return An recycled or new IndexID or'd with the generator's GroupID and the TypeID for
     * resources.
     */
    FOE_ECS_EXPORT foeId generateResource();

    /** Frees an IndexID so that it may be recycled.
     * @param id The ID to recycle.
     * @return True if freed, false otherwise.
     *
     * EntityID must be from the same GroupID.
     */
    FOE_ECS_EXPORT bool free(foeId id);

    /** Frees an IndexID so that it may be recycled.
     * @param idList Set of IDs to be freed.
     * @return True if freed, false otherwise.
     *
     * ExternalIDs must be from the same BaseID.
     */
    FOE_ECS_EXPORT bool free(uint32_t count, foeId *pEntities);

    /// Returns the name of the group
    FOE_ECS_EXPORT std::string_view name() const noexcept;

    /** Returns the group id for the class.
     * @return The class's group id.
     */
    FOE_ECS_EXPORT foeIdGroup groupID() const noexcept;

    /** Peeks at the next *fresh* index that will be generated.
     * @return The next fresh index that will be generated.
     * @warning Do NOT use this for new IDs. Use the generate() function to do that. This is for
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

    /**
     * @brief Overwrites the current state of the generator with that provided
     * @param nextIndex Next free index that will be generated
     * @param recycledIndices The new set of recycled indices
     * @warning Overwrites the current state of the object
     */
    FOE_ECS_EXPORT void importState(foeIdIndex nextIndex,
                                    std::vector<foeIdIndex> const &recycledIndices);

    /**
     * @brief Returns the current state of the object for import/export
     * @param[out] nextIndex Next free index
     * @param[out] recycledIndices The set of recycled indices
     * @warning Overwrites the current state of the object
     */
    FOE_ECS_EXPORT void exportState(foeIdIndex &nextIndex,
                                    std::vector<foeIdIndex> &recycledIndices);

    /**
     * @brief Returns a list of all the currently 'active' entities
     * @return The full list of 'active' entities
     *
     * 'Active' entities are those that are below the current next free index, and haven't been
     * 'recycled'.
     */
    FOE_ECS_EXPORT auto activeEntityList() -> std::vector<foeId>;

  private:
    /// Name of the group
    std::string const cName;
    /// The group of the entity IDs being managed
    foeIdGroup const cGroupID;

    /// Synchronizes the recycle list
    std::mutex mSync;
    /// The next free IndexID, never yet used.
    foeIdIndex mNextFreeID;
    /// The list of recyclable IndexIDs.
    std::queue<foeIdIndex> mRecycled;
    /// The number of currently recyclable IDs.
    size_t mNumRecycled;
};

#endif // FOE_ECS_INDEX_GENERATOR_HPP