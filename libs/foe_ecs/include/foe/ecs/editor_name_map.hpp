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

#ifndef FOE_ECS_EDITOR_NAME_MAP_HPP
#define FOE_ECS_EDITOR_NAME_MAP_HPP

#include <foe/ecs/id.hpp>
#include <foe/export.h>

#include <map>
#include <shared_mutex>
#include <string>

/** @brief A two-way map between an ID and a human-readable string
 *
 * In development environments, using exclusively numeric or hexadecimal values to identify
 * everything is great for many unique elements, but absolute havoc when trying to remember of
 * search for things throughout the simulation.
 *
 * To make things easier for development, this can be added that allows for a two-way map between
 * IDs and strings.
 *
 * To aid this, all operations are synchronized through a shared mutex, meaning operations on this
 * are slower, thus why it is meant only for development purposes, however to speed things up
 * slightly there are maps for going both ways.
 *
 * It is also meant to be universal for a simulation, ie. each ID or editorName must be unique in
 * the map and, hopefully, simulation.
 *
 * @note Thread-safe
 */
struct foeEditorNameMap {
  public:
    /** @brief Finds an ID from the given string
     * @param editorName String to find items with
     * @return The ID that uses that name. If no item does, returns FOE_NULL_HANDLE.
     */
    FOE_EXPORT foeId find(std::string_view editorName);

    /** @brief Finds the editorName string from the given ID
     * @param id ID to match
     * @return editorName, or if no match, an empty string
     */
    FOE_EXPORT std::string find(foeId id);

    /** @brief Attempts to add a new ID/editorName pairing
     * @param id Unique ID to add.
     * @param editorName Unique EditorName to add
     * @return True if it was added. Returns false if either the ID or name is not unique to the
     * map, or the name was empty.
     */
    FOE_EXPORT bool add(foeId id, std::string editorName);

    /** @brief Updates an editorName for the given ID
     * @param id ID to update
     * @param editorName New EditorName for the ID
     * @return True if update was successful. If the ID does not exist in the map, or name is not
     * unique to the map, or the new name is empty, returns false.
     */
    FOE_EXPORT bool update(foeId id, std::string editorName);

    /** @brief Removes an ID/editorName pair from the map.
     * @param id ID to find and remove for
     * @return True if successful. If the ID isn't in the map, returns false.
     */
    FOE_EXPORT bool remove(foeId id);

  private:
    /// Synchronizes access to the maps. Exclusive when modifying the maps, shared when just reading
    std::shared_mutex mSync{};

    std::map<std::string, foeId> mEditorToId;
    std::map<foeId, std::string> mIdToEditor;
};

#endif // FOE_ECS_EDITOR_NAME_MAP_HPP