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

#ifndef FOE_ECS_ID_TO_STRING_HPP
#define FOE_ECS_ID_TO_STRING_HPP

#include <foe/ecs/export.h>
#include <foe/ecs/id.h>

#include <string>

/** @brief Converts the foeId to a more readable hexadecimal string
 * @param id ID to convert
 * @return The ID in a hexadecimal format
 */
FOE_ECS_EXPORT std::string foeIdToString(foeId id);

/** @brief Converts the foeIdGroup to a more readable hexadecimal string
 * @param id ID to convert
 * @return The ID in a hexadecimal format
 */
FOE_ECS_EXPORT std::string foeIdGroupToString(foeIdGroup group);

/** @brief Converts the foeIdIndex to a more readable hexadecimal string
 * @param id ID to convert
 * @return The ID in a hexadecimal format
 */
FOE_ECS_EXPORT std::string foeIdIndexToString(foeIdIndex index);

/** @brief Converts the foeId to a more readable hexadecimal string, witht he different parts split
 * @param id ID to convert
 * @return The ID in a split-hexadecimal format
 *
 * Each of the constituent parts of the ID are split via '-', with the format being
 * Group-Index.
 */
FOE_ECS_EXPORT std::string foeIdToSplitString(foeId id);

#endif // FOE_ECS_ID_TO_STRING_HPP