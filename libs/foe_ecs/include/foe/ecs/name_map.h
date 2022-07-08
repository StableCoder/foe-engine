// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_ECS_NAME_MAP_H
#define FOE_ECS_NAME_MAP_H

#include <foe/ecs/export.h>
#include <foe/ecs/id.h>
#include <foe/error_code.h>
#include <foe/handle.h>

#ifdef __cplusplus
extern "C" {
#endif

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
FOE_DEFINE_HANDLE(foeEcsNameMap)

FOE_ECS_EXPORT foeResult foeEcsCreateNameMap(foeEcsNameMap *pNameMap);

FOE_ECS_EXPORT void foeEcsDestroyNameMap(foeEcsNameMap nameMap);

FOE_ECS_EXPORT foeResult foeEcsNameMapFindID(foeEcsNameMap nameMap, char const *pName, foeId *pID);

FOE_ECS_EXPORT foeResult foeEcsNameMapFindName(foeEcsNameMap nameMap,
                                               foeId id,
                                               uint32_t *pNameLength,
                                               char *pName);

FOE_ECS_EXPORT foeResult foeEcsNameMapAdd(foeEcsNameMap nameMap, foeId id, char const *pName);

FOE_ECS_EXPORT foeResult foeEcsNameMapUpdate(foeEcsNameMap nameMap, foeId id, char const *pName);

FOE_ECS_EXPORT foeResult foeEcsNameMapRemove(foeEcsNameMap nameMap, foeId id);

#ifdef __cplusplus
}
#endif

#endif // FOE_ECS_NAME_MAP_H