// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_SIMULATION_RESOURCE_CREATE_INFO_POOL_H
#define FOE_SIMULATION_RESOURCE_CREATE_INFO_POOL_H

#include <foe/ecs/id.h>
#include <foe/handle.h>
#include <foe/resource/create_info.h>
#include <foe/result.h>
#include <foe/simulation/export.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeResourceCreateInfoPool)

FOE_SIM_EXPORT
foeResultSet foeCreateResourceCreateInfoPool(foeResourceCreateInfoPool *pResourceCreateInfoPool);

FOE_SIM_EXPORT
void foeDestroyResourceCreateInfoPool(foeResourceCreateInfoPool resourceCreateInfoPool);

FOE_SIM_EXPORT
foeResultSet foeResourceCreateInfoPoolAdd(foeResourceCreateInfoPool resourceCreateInfoPool,
                                          foeResourceID resourceID,
                                          foeResourceCreateInfo resourceCreateInfo);

FOE_SIM_EXPORT
foeResultSet foeResourceCreateInfoPoolRemove(foeResourceCreateInfoPool resourceCreateInfoPool,
                                             foeResourceID resourceID);

// Returned handles are pre-incremented
FOE_SIM_EXPORT
foeResourceCreateInfo foeResourceCreateInfoPoolGet(foeResourceCreateInfoPool resourceCreateInfoPool,
                                                   foeResourceID resourceID);

#ifdef __cplusplus
}
#endif

#endif // FOE_SIMULATION_RESOURCE_CREATE_INFO_POOL_H