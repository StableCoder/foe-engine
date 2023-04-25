// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_SIMULATION_RESOURCE_CREATE_INFO_HISTORY_H
#define FOE_SIMULATION_RESOURCE_CREATE_INFO_HISTORY_H

#include <foe/ecs/id.h>
#include <foe/handle.h>
#include <foe/resource/create_info.h>
#include <foe/result.h>
#include <foe/simulation/export.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeResourceCreateInfoHistory)

FOE_SIM_EXPORT
foeResultSet foeCreateResourceCreateInfoHistory(
    foeResourceCreateInfoHistory *pResourceCreateInfoHistory);

FOE_SIM_EXPORT
void foeDestroyResourceCreateInfoHistory(foeResourceCreateInfoHistory resourceCreateInfoHistory);

FOE_SIM_EXPORT
foeResultSet foeResourceCreateInfoHistoryAdd(foeResourceCreateInfoHistory resourceCreateInfoHistory,
                                             foeResourceID resourceID,
                                             foeResourceCreateInfo resourceCreateInfo);

FOE_SIM_EXPORT
foeResultSet foeResourceCreateInfoHistoryRemove(
    foeResourceCreateInfoHistory resourceCreateInfoHistory, foeResourceID resourceID);

FOE_SIM_EXPORT
foeResultSet foeResourceCreateInfoHistoryUpdate(
    foeResourceCreateInfoHistory resourceCreateInfoHistory,
    foeResourceID resourceID,
    foeResourceCreateInfo resourceCreateInfo);

FOE_SIM_EXPORT
foeResourceCreateInfo foeResourceCreateInfoHistoryCurrent(
    foeResourceCreateInfoHistory resourceCreateInfoHistory, foeResourceID resourceID);

FOE_SIM_EXPORT
foeResultSet foeResourceCreateInfoHistoryUndo(
    foeResourceCreateInfoHistory resourceCreateInfoHistory, foeResourceID resourceID);

FOE_SIM_EXPORT
foeResultSet foeResourceCreateInfoHistoryRedo(
    foeResourceCreateInfoHistory resourceCreateInfoHistory, foeResourceID resourceID);

#ifdef __cplusplus
}
#endif

#endif // FOE_SIMULATION_RESOURCE_CREATE_INFO_HISTORY_H