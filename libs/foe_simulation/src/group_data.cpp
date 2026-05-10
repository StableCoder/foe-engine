// Copyright (C) 2021-2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/simulation/group_data.h>

#include "log.hpp"
#include "result.h"

#include <array>

namespace {

std::string_view constexpr cPersistentName = "Persistent";
std::string_view constexpr cTemporaryName = "Temporary";

struct GroupData {
    struct CombinedGroup {
        foeEcsIndexes entityIndexes{FOE_NULL_HANDLE};
        foeEcsIndexes resourceIndexes{FOE_NULL_HANDLE};
        foeImexImporter importer{FOE_NULL_HANDLE};

        ~CombinedGroup();
    };

    foeEcsIndexes mPersistentEntityIndexes{FOE_NULL_HANDLE};
    foeEcsIndexes mPersistentResourceIndexes{FOE_NULL_HANDLE};
    foeImexImporter mPersistentImporter{FOE_NULL_HANDLE};

    foeEcsIndexes mTemporaryEntityIndexes{FOE_NULL_HANDLE};
    foeEcsIndexes mTemporaryResourceIndexes{FOE_NULL_HANDLE};

    std::array<CombinedGroup, foeIdNumDynamicGroups> mDynamicGroups;
};

GroupData::CombinedGroup::~CombinedGroup() {
    if (importer != FOE_NULL_HANDLE)
        foeDestroyImporter(importer);
    if (resourceIndexes != FOE_NULL_HANDLE)
        foeEcsDestroyIndexes(resourceIndexes);
    if (entityIndexes != FOE_NULL_HANDLE)
        foeEcsDestroyIndexes(entityIndexes);
}

FOE_DEFINE_HANDLE_CASTS(group_data, GroupData, foeGroupData)

} // namespace

extern "C" foeResultSet foeSimulationCreateGroupData(foeGroupData *pGroupData) {
    GroupData *pNewGroupData = new (std::nothrow) GroupData;
    if (pNewGroupData == nullptr)
        return to_foeResult(FOE_SIMULATION_ERROR_OUT_OF_MEMORY);

    foeResultSet result;

    result = foeEcsCreateIndexes(foeIdPersistentGroup, &pNewGroupData->mPersistentEntityIndexes);
    if (result.value != FOE_SUCCESS)
        goto GROUP_DATA_CREATE_FAILED;
    result = foeEcsCreateIndexes(foeIdPersistentGroup, &pNewGroupData->mPersistentResourceIndexes);
    if (result.value != FOE_SUCCESS)
        goto GROUP_DATA_CREATE_FAILED;

    result = foeEcsCreateIndexes(foeIdTemporaryGroup, &pNewGroupData->mTemporaryEntityIndexes);
    if (result.value != FOE_SUCCESS)
        goto GROUP_DATA_CREATE_FAILED;
    result = foeEcsCreateIndexes(foeIdTemporaryGroup, &pNewGroupData->mTemporaryResourceIndexes);
    if (result.value != FOE_SUCCESS)
        goto GROUP_DATA_CREATE_FAILED;

    *pGroupData = group_data_to_handle(pNewGroupData);

    return to_foeResult(FOE_SIMULATION_SUCCESS);
GROUP_DATA_CREATE_FAILED:
    foeSimulationDestroyGroupData(group_data_to_handle(pNewGroupData));

    return result;
}

extern "C" void foeSimulationDestroyGroupData(foeGroupData groupData) {
    GroupData *pGroupData = group_data_from_handle(groupData);

    if (pGroupData->mTemporaryResourceIndexes != FOE_NULL_HANDLE)
        foeEcsDestroyIndexes(pGroupData->mTemporaryResourceIndexes);
    if (pGroupData->mTemporaryEntityIndexes != FOE_NULL_HANDLE)
        foeEcsDestroyIndexes(pGroupData->mTemporaryEntityIndexes);

    if (pGroupData->mPersistentResourceIndexes != FOE_NULL_HANDLE)
        foeEcsDestroyIndexes(pGroupData->mPersistentResourceIndexes);
    if (pGroupData->mPersistentEntityIndexes != FOE_NULL_HANDLE)
        foeEcsDestroyIndexes(pGroupData->mPersistentEntityIndexes);

    if (pGroupData->mPersistentImporter != FOE_NULL_HANDLE)
        foeDestroyImporter(pGroupData->mPersistentImporter);

    delete pGroupData;
}

extern "C" bool foeSimulationAddDynamicGroup(foeGroupData groupData,
                                             foeEcsIndexes entityIndexes,
                                             foeEcsIndexes resourceIndexes,
                                             foeImexImporter importer) {
    GroupData *pGroupData = group_data_from_handle(groupData);

    // Make sure both items are valid pointers
    if (entityIndexes == FOE_NULL_HANDLE || resourceIndexes == FOE_NULL_HANDLE ||
        importer == FOE_NULL_HANDLE) {
        FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                "foeSimulationAddDynamicGroup - Either the given indexes or importer are nullptr");
        return false;
    }

    char const *pGroupName;
    foeResultSet result = foeImexImporterGetGroupName(importer, &pGroupName);
    if (result.value != FOE_SUCCESS) {
        return false;
    }

    // Check against blank name for the importer
    if (std::string_view{pGroupName}.empty()) {
        FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                "foeSimulationAddDynamicGroup - Importer had a blank group name");
        return false;
    }

    foeIdGroup groupID;
    result = foeImexImporterGetGroupID(importer, &groupID);
    if (result.value != FOE_SUCCESS) {
        return false;
    }

    // Check both have the same ID Group
    if (foeEcsIndexesGetGroupID(entityIndexes) != groupID ||
        foeEcsIndexesGetGroupID(resourceIndexes) != groupID) {
        FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                "foeSimulationAddDynamicGroup - ID Groups don't match between the indexes and "
                "importer");
        return false;
    }

    // Must be within the dynamic groups valid range
    auto groupValue = foeIdGroupToValue(foeEcsIndexesGetGroupID(entityIndexes));
    if (groupValue >= foeIdNumDynamicGroups) {
        FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                "foeSimulationAddDynamicGroup - ID Group is not within the valid dynamic group "
                "value range");
        return false;
    }

    // Check that the group isn't already used
    if (pGroupData->mDynamicGroups[groupValue].entityIndexes != FOE_NULL_HANDLE) {
        FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                "foeSimulationAddDynamicGroup - Attempted to add ID group that is already used")
        return false;
    }

    // Check against duplicate names
    if (strcmp(pGroupName, cPersistentName.data()) == 0 ||
        strcmp(pGroupName, cTemporaryName.data()) == 0) {
        FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                "foeSimulationAddDynamicGroup - Importer group name is a reserved name, either "
                "'Persistent' or 'Temporary'");
        return false;
    }

    for (auto const &it : pGroupData->mDynamicGroups) {
        if (it.importer == FOE_NULL_HANDLE)
            continue;

        char const *pDynamicGroupName;
        result = foeImexImporterGetGroupName(it.importer, &pDynamicGroupName);
        if (result.value != FOE_SUCCESS) {
            continue;
        }

        if (strcmp(pDynamicGroupName, pGroupName) == 0) {
            FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                    "foeSimulationAddDynamicGroup - Importer group name already exists");
            return false;
        }
    }

    pGroupData->mDynamicGroups[groupValue].entityIndexes = entityIndexes;
    pGroupData->mDynamicGroups[groupValue].resourceIndexes = resourceIndexes;
    pGroupData->mDynamicGroups[groupValue].importer = importer;

    return true;
}

bool foeSimulationSetPersistentImporter(foeGroupData groupData, foeImexImporter importer) {
    GroupData *pGroupData = group_data_from_handle(groupData);

    if (importer == FOE_NULL_HANDLE) {
        FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                "foeSimulationSetPersistentImporter - Importer group not given (nullptr)");
        return false;
    }

    foeIdGroup group;
    foeResultSet result = foeImexImporterGetGroupID(importer, &group);
    if (result.value != FOE_SUCCESS)
        return false;

    if (group != foeIdPersistentGroup) {
        FOE_LOG(
            foeSimulation, FOE_LOG_LEVEL_ERROR,
            "foeSimulationSetPersistentImporter - Importer group given not foeIdPersistentGroup");
        return false;
    }

    if (pGroupData->mPersistentImporter != FOE_NULL_HANDLE) {
        foeDestroyImporter(pGroupData->mPersistentImporter);
    }

    pGroupData->mPersistentImporter = importer;

    return true;
}

extern "C" foeEcsIndexes foeSimulationEntityIndexes(foeGroupData groupData, foeIdGroup group) {
    GroupData *pGroupData = group_data_from_handle(groupData);

    auto idGroup = foeIdGetGroup(group);

    if (idGroup == foeIdPersistentGroup) {
        return foeSimulationPersistentEntityIndexes(groupData);
    } else if (idGroup == foeIdTemporaryGroup) {
        return foeSimulationTemporaryEntityIndexes(groupData);
    }

    return pGroupData->mDynamicGroups[foeIdGroupToValue(idGroup)].entityIndexes;
}

extern "C" foeEcsIndexes foeSimulationResourceIndexes(foeGroupData groupData, foeIdGroup group) {
    GroupData *pGroupData = group_data_from_handle(groupData);

    auto idGroup = foeIdGetGroup(group);

    if (idGroup == foeIdPersistentGroup) {
        return foeSimulationPersistentResourceIndexes(groupData);
    } else if (idGroup == foeIdTemporaryGroup) {
        return foeSimulationTemporaryResourceIndexes(groupData);
    }

    return pGroupData->mDynamicGroups[foeIdGroupToValue(idGroup)].resourceIndexes;
}

extern "C" foeImexImporter foeSimulationImporter(foeGroupData groupData, foeIdGroup group) {
    GroupData *pGroupData = group_data_from_handle(groupData);

    auto idGroup = foeIdGetGroup(group);

    if (idGroup == foeIdPersistentGroup) {
        return foeSimulationPersistentImporter(groupData);
    } else if (idGroup == foeIdTemporaryGroup) {
        return FOE_NULL_HANDLE;
    }

    return pGroupData->mDynamicGroups[foeIdGroupToValue(idGroup)].importer;
}

extern "C" foeEcsIndexes foeSimulationPersistentEntityIndexes(foeGroupData groupData) {
    GroupData *pGroupData = group_data_from_handle(groupData);

    return pGroupData->mPersistentEntityIndexes;
}

extern "C" foeEcsIndexes foeSimulationPersistentResourceIndexes(foeGroupData groupData) {
    GroupData *pGroupData = group_data_from_handle(groupData);

    return pGroupData->mPersistentResourceIndexes;
}

extern "C" foeImexImporter foeSimulationPersistentImporter(foeGroupData groupData) {
    GroupData *pGroupData = group_data_from_handle(groupData);

    return pGroupData->mPersistentImporter;
}

extern "C" foeEcsIndexes foeSimulationTemporaryEntityIndexes(foeGroupData groupData) {
    GroupData *pGroupData = group_data_from_handle(groupData);

    return pGroupData->mTemporaryEntityIndexes;
}

extern "C" foeEcsIndexes foeSimulationTemporaryResourceIndexes(foeGroupData groupData) {
    GroupData *pGroupData = group_data_from_handle(groupData);

    return pGroupData->mTemporaryResourceIndexes;
}

extern "C" foeResourceCreateInfo foeSimulationGetGroupDataResourceCreateInfo(foeGroupData groupData,
                                                                             foeId id) {
    GroupData *pGroupData = group_data_from_handle(groupData);

    if (pGroupData->mPersistentImporter != nullptr) {
        foeResourceCreateInfo createInfo = FOE_NULL_HANDLE;
        foeResultSet result =
            foeImexImporterGetResourceCreateInfo(pGroupData->mPersistentImporter, id, &createInfo);
        if (result.value == FOE_SUCCESS && createInfo != FOE_NULL_HANDLE) {
            return createInfo;
        }
    }

    for (auto it = pGroupData->mDynamicGroups.rbegin(); it != pGroupData->mDynamicGroups.rend();
         ++it) {
        if (it->importer == FOE_NULL_HANDLE)
            continue;

        foeResourceCreateInfo createInfo = FOE_NULL_HANDLE;
        foeResultSet result = foeImexImporterGetResourceCreateInfo(it->importer, id, &createInfo);
        if (result.value == FOE_SUCCESS && createInfo != FOE_NULL_HANDLE) {
            return createInfo;
        }
    }

    return FOE_NULL_HANDLE;
}

extern "C" foeResultSet foeSimulationFindExternalFile(foeGroupData groupData,
                                                      char const *pFilePath,
                                                      foeManagedMemory *pManagedMemory) {
    GroupData *pGroupData = group_data_from_handle(groupData);

    if (pGroupData->mPersistentImporter != nullptr) {
        foeResultSet result = foeImexImporterFindExternalFile(pGroupData->mPersistentImporter,
                                                              pFilePath, pManagedMemory);
        if (result.value == FOE_SUCCESS) {
            return result;
        }
    }

    for (auto it = pGroupData->mDynamicGroups.rbegin(); it != pGroupData->mDynamicGroups.rend();
         ++it) {
        if (it->importer == FOE_NULL_HANDLE)
            continue;

        foeResultSet result =
            foeImexImporterFindExternalFile(it->importer, pFilePath, pManagedMemory);
        if (result.value == FOE_SUCCESS) {
            return result;
        }
    }

    return to_foeResult(FOE_SIMULATION_ERROR_CONTENT_NOT_FOUND);
}