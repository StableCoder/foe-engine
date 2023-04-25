// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_ECS_ID_TO_STRING_HPP
#define FOE_ECS_ID_TO_STRING_HPP

#include <foe/ecs/export.h>
#include <foe/ecs/id.h>

#include <string>

/** @brief Converts the foeId to a more readable hexadecimal string
 * @param id ID to convert
 * @return The ID in a hexadecimal format
 */
FOE_ECS_EXPORT
std::string foeIdToString(foeId id);

/** @brief Converts the foeIdGroup to a more readable hexadecimal string
 * @param id ID to convert
 * @return The ID in a hexadecimal format
 */
FOE_ECS_EXPORT
std::string foeIdGroupToString(foeIdGroup group);

/** @brief Converts the foeIdIndex to a more readable hexadecimal string
 * @param id ID to convert
 * @return The ID in a hexadecimal format
 */
FOE_ECS_EXPORT
std::string foeIdIndexToString(foeIdIndex index);

#endif // FOE_ECS_ID_TO_STRING_HPP