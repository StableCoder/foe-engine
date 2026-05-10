// Copyright (C) 2021-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_SIMULATION_GROUP_DATA_H
#define FOE_SIMULATION_GROUP_DATA_H

#include <foe/ecs/indexes.h>
#include <foe/handle.h>
#include <foe/imex/importer.h>
#include <foe/result.h>
#include <foe/simulation/export.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeGroupData)

FOE_SIM_EXPORT
foeResultSet foeSimulationCreateGroupData(foeGroupData *pGroupData);

FOE_SIM_EXPORT
void foeSimulationDestroyGroupData(foeGroupData groupData);

FOE_SIM_EXPORT
bool foeSimulationAddDynamicGroup(foeGroupData groupData,
                                  foeEcsIndexes entityIndexes,
                                  foeEcsIndexes resourceIndexes,
                                  foeImexImporter importer);

FOE_SIM_EXPORT
bool foeSimulationSetPersistentImporter(foeGroupData groupData, foeImexImporter importer);

FOE_SIM_EXPORT
foeEcsIndexes foeSimulationEntityIndexes(foeGroupData groupData, foeIdGroup group);

FOE_SIM_EXPORT
foeEcsIndexes foeSimulationResourceIndexes(foeGroupData groupData, foeIdGroup group);

FOE_SIM_EXPORT
foeImexImporter foeSimulationImporter(foeGroupData groupData, foeIdGroup group);

FOE_SIM_EXPORT
foeEcsIndexes foeSimulationPersistentEntityIndexes(foeGroupData groupData);

FOE_SIM_EXPORT
foeEcsIndexes foeSimulationPersistentResourceIndexes(foeGroupData groupData);

FOE_SIM_EXPORT
foeImexImporter foeSimulationPersistentImporter(foeGroupData groupData);

FOE_SIM_EXPORT
foeEcsIndexes foeSimulationTemporaryEntityIndexes(foeGroupData groupData);

FOE_SIM_EXPORT
foeEcsIndexes foeSimulationTemporaryResourceIndexes(foeGroupData groupData);

FOE_SIM_EXPORT
foeResourceCreateInfo foeSimulationGetGroupDataResourceCreateInfo(foeGroupData groupData, foeId id);

FOE_SIM_EXPORT
foeResultSet foeSimulationFindExternalFile(foeGroupData groupData,
                                           char const *pFilePath,
                                           foeManagedMemory *pManagedMemory);

#ifdef __cplusplus
}
#endif

#endif // FOE_SIMULATION_GROUP_DATA_H