// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/simulation/group_data.hpp>

#include "log.hpp"
#include "result.h"

foeGroupData::CombinedGroup::~CombinedGroup() {
    if (importer != FOE_NULL_HANDLE)
        foeDestroyImporter(importer);
    if (resourceIndexes != FOE_NULL_HANDLE)
        foeEcsDestroyIndexes(resourceIndexes);
    if (entityIndexes != FOE_NULL_HANDLE)
        foeEcsDestroyIndexes(entityIndexes);
}

foeGroupData::foeGroupData() {
    foeResultSet result;

    result = foeEcsCreateIndexes(foeIdPersistentGroup, &mPersistentEntityIndexes);
    result = foeEcsCreateIndexes(foeIdPersistentGroup, &mPersistentResourceIndexes);

    result = foeEcsCreateIndexes(foeIdTemporaryGroup, &mTemporaryEntityIndexes);
    result = foeEcsCreateIndexes(foeIdTemporaryGroup, &mTemporaryResourceIndexes);
}

foeGroupData::~foeGroupData() {
    if (mTemporaryResourceIndexes != FOE_NULL_HANDLE)
        foeEcsDestroyIndexes(mTemporaryResourceIndexes);
    if (mTemporaryEntityIndexes != FOE_NULL_HANDLE)
        foeEcsDestroyIndexes(mTemporaryEntityIndexes);

    if (mPersistentResourceIndexes != FOE_NULL_HANDLE)
        foeEcsDestroyIndexes(mPersistentResourceIndexes);
    if (mPersistentEntityIndexes != FOE_NULL_HANDLE)
        foeEcsDestroyIndexes(mPersistentEntityIndexes);

    if (mPersistentImporter != FOE_NULL_HANDLE)
        foeDestroyImporter(mPersistentImporter);
}

bool foeGroupData::addDynamicGroup(foeEcsIndexes entityIndexes,
                                   foeEcsIndexes resourceIndexes,
                                   foeImexImporter importer) {
    // Make sure both items are valid pointers
    if (entityIndexes == FOE_NULL_HANDLE || resourceIndexes == FOE_NULL_HANDLE ||
        importer == FOE_NULL_HANDLE) {
        FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                "foeGroupData::addDynamicGroup - Either the given indexes or importer are nullptr");
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
                "foeGroupData::addDynamicGroup - Importer had a blank group name");
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
                "foeGroupData::addDynamicGroup - ID Groups don't match between the indexes and "
                "importer");
        return false;
    }

    // Must be within the dynamic groups valid range
    auto groupValue = foeIdGroupToValue(foeEcsIndexesGetGroupID(entityIndexes));
    if (groupValue >= foeIdNumDynamicGroups) {
        FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                "foeGroupData::addDynamicGroup - ID Group is not within the valid dynamic group "
                "value range");
        return false;
    }

    // Check that the group isn't already used
    if (mDynamicGroups[groupValue].entityIndexes != FOE_NULL_HANDLE) {
        FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                "foeGroupData::addDynamicGroup - Attempted to add ID group that is already used")
        return false;
    }

    // Check against duplicate names
    if (strcmp(pGroupName, cPersistentName.data()) == 0 ||
        strcmp(pGroupName, cTemporaryName.data()) == 0) {
        FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                "foeGroupData::addDynamicGroup - Importer group name is a reserved name, either "
                "'Persistent' or 'Temporary'");
        return false;
    }

    for (auto const &it : mDynamicGroups) {
        if (it.importer == FOE_NULL_HANDLE)
            continue;

        char const *pDynamicGroupName;
        result = foeImexImporterGetGroupName(it.importer, &pDynamicGroupName);
        if (result.value != FOE_SUCCESS) {
            continue;
        }

        if (strcmp(pDynamicGroupName, pGroupName) == 0) {
            FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                    "foeGroupData::addDynamicGroup - Importer group name already exists");
            return false;
        }
    }

    mDynamicGroups[groupValue].entityIndexes = entityIndexes;
    mDynamicGroups[groupValue].resourceIndexes = resourceIndexes;
    mDynamicGroups[groupValue].importer = importer;
    return true;
}

bool foeGroupData::setPersistentImporter(foeImexImporter importer) {
    if (importer == FOE_NULL_HANDLE) {
        FOE_LOG(foeSimulation, FOE_LOG_LEVEL_ERROR,
                "foeGroupData::setPersistentImporter - Importer group not given (nullptr)");
        return false;
    }

    foeIdGroup group;
    foeResultSet result = foeImexImporterGetGroupID(importer, &group);
    if (result.value != FOE_SUCCESS)
        return false;

    if (group != foeIdPersistentGroup) {
        FOE_LOG(
            foeSimulation, FOE_LOG_LEVEL_ERROR,
            "foeGroupData::setPersistentImporter - Importer group given not foeIdPersistentGroup");
        return false;
    }

    if (mPersistentImporter != FOE_NULL_HANDLE) {
        foeDestroyImporter(mPersistentImporter);
    }

    mPersistentImporter = importer;
    return true;
}

foeEcsIndexes foeGroupData::entityIndexes(foeIdGroup group) noexcept {
    auto idGroup = foeIdGetGroup(group);

    if (idGroup == foeIdPersistentGroup) {
        return persistentEntityIndexes();
    } else if (idGroup == foeIdTemporaryGroup) {
        return temporaryEntityIndexes();
    }

    return mDynamicGroups[foeIdGroupToValue(idGroup)].entityIndexes;
}

foeEcsIndexes foeGroupData::entityIndexes(std::string_view groupName) noexcept {
    if (groupName == cPersistentName) {
        return persistentEntityIndexes();
    } else if (groupName == cTemporaryName) {
        return temporaryEntityIndexes();
    }

    for (auto const &it : mDynamicGroups) {
        if (it.importer == FOE_NULL_HANDLE)
            continue;

        char const *pDynamicGroupName;
        foeResultSet result = foeImexImporterGetGroupName(it.importer, &pDynamicGroupName);
        if (result.value != FOE_SUCCESS) {
            continue;
        }

        if (std::string_view{pDynamicGroupName} == groupName) {
            return it.entityIndexes;
        }
    }

    return nullptr;
}

foeEcsIndexes foeGroupData::resourceIndexes(foeIdGroup group) noexcept {
    auto idGroup = foeIdGetGroup(group);

    if (idGroup == foeIdPersistentGroup) {
        return persistentResourceIndexes();
    } else if (idGroup == foeIdTemporaryGroup) {
        return temporaryResourceIndexes();
    }

    return mDynamicGroups[foeIdGroupToValue(idGroup)].resourceIndexes;
}

foeEcsIndexes foeGroupData::resourceIndexes(std::string_view groupName) noexcept {
    if (groupName == cPersistentName) {
        return persistentResourceIndexes();
    } else if (groupName == cTemporaryName) {
        return temporaryResourceIndexes();
    }

    for (auto const &it : mDynamicGroups) {
        if (it.importer == nullptr)
            continue;

        char const *pDynamicGroupName;
        foeResultSet result = foeImexImporterGetGroupName(it.importer, &pDynamicGroupName);
        if (result.value != FOE_SUCCESS) {
            continue;
        }

        if (std::string_view{pDynamicGroupName} == groupName) {
            return it.resourceIndexes;
        }
    }

    return nullptr;
}

auto foeGroupData::importer(foeIdGroup group) noexcept -> foeImexImporter {
    auto idGroup = foeIdGetGroup(group);

    if (idGroup == foeIdPersistentGroup) {
        return persistentImporter();
    } else if (idGroup == foeIdTemporaryGroup) {
        return nullptr;
    }

    return mDynamicGroups[foeIdGroupToValue(idGroup)].importer;
}

auto foeGroupData::importer(std::string_view groupName) noexcept -> foeImexImporter {
    char const *pGroupName;

    if (groupName == cTemporaryName) {
        return nullptr;
    } else if (groupName == cPersistentName) {
        return persistentImporter();
    } else if (mPersistentImporter != nullptr) {
        foeResultSet result = foeImexImporterGetGroupName(mPersistentImporter, &pGroupName);
        if (result.value == FOE_SUCCESS && std::string_view{pGroupName} == groupName) {
            return persistentImporter();
        }
    }

    for (auto const &it : mDynamicGroups) {
        if (it.importer == FOE_NULL_HANDLE)
            continue;

        foeResultSet result = foeImexImporterGetGroupName(it.importer, &pGroupName);
        if (result.value != FOE_SUCCESS) {
            continue;
        }

        if (std::string_view{pGroupName} == groupName) {
            return it.importer;
        }
    }

    return nullptr;
}

foeEcsIndexes foeGroupData::persistentEntityIndexes() noexcept { return mPersistentEntityIndexes; }

foeEcsIndexes foeGroupData::persistentResourceIndexes() noexcept {
    return mPersistentResourceIndexes;
}

auto foeGroupData::persistentImporter() noexcept -> foeImexImporter { return mPersistentImporter; }

auto foeGroupData::temporaryEntityIndexes() noexcept -> foeEcsIndexes {
    return mTemporaryEntityIndexes;
}

auto foeGroupData::temporaryResourceIndexes() noexcept -> foeEcsIndexes {
    return mTemporaryResourceIndexes;
}

foeResourceCreateInfo foeGroupData::getResourceCreateInfo(foeId id) {
    if (mPersistentImporter != nullptr) {
        foeResourceCreateInfo createInfo = FOE_NULL_HANDLE;
        foeResultSet result =
            foeImexImporterGetResourceCreateInfo(mPersistentImporter, id, &createInfo);
        if (result.value == FOE_SUCCESS && createInfo != FOE_NULL_HANDLE) {
            return createInfo;
        }
    }

    for (auto it = mDynamicGroups.rbegin(); it != mDynamicGroups.rend(); ++it) {
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

foeResultSet foeGroupData::findExternalFile(char const *pFilePath,
                                            foeManagedMemory *pManagedMemory) {
    if (mPersistentImporter != nullptr) {
        foeResultSet result =
            foeImexImporterFindExternalFile(mPersistentImporter, pFilePath, pManagedMemory);
        if (result.value == FOE_SUCCESS) {
            return result;
        }
    }

    for (auto it = mDynamicGroups.rbegin(); it != mDynamicGroups.rend(); ++it) {
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