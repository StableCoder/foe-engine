// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/simulation/group_data.hpp>

#include "log.hpp"

foeGroupData::CombinedGroup::~CombinedGroup() {
    if (resourceIndexes != FOE_NULL_HANDLE)
        foeEcsDestroyIndexes(resourceIndexes);
    if (entityIndexes != FOE_NULL_HANDLE)
        foeEcsDestroyIndexes(entityIndexes);
}

foeGroupData::foeGroupData() {
    foeResult result;

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
}

bool foeGroupData::addDynamicGroup(foeEcsIndexes entityIndexes,
                                   foeEcsIndexes resourceIndexes,
                                   std::unique_ptr<foeImporterBase> &&pImporter) {
    // Make sure both items are valid pointers
    if (entityIndexes == FOE_NULL_HANDLE || resourceIndexes == FOE_NULL_HANDLE ||
        pImporter == nullptr) {
        FOE_LOG(SimulationState, Error,
                "foeGroupData::addDynamicGroup - Either the given indexes or importer are nullptr");
        return false;
    }

    char const *pGroupName;
    foeResult result = pImporter->name(&pGroupName);
    if (result.value != FOE_SUCCESS) {
        return false;
    }

    // Check against blank name for the importer
    if (std::string_view{pGroupName}.empty()) {
        FOE_LOG(SimulationState, Error,
                "foeGroupData::addDynamicGroup - Importer had a blank group name");
        return false;
    }

    foeIdGroup groupID;
    result = pImporter->group(&groupID);
    if (result.value != FOE_SUCCESS) {
        return false;
    }

    // Check both have the same ID Group
    if (foeEcsIndexesGetGroupID(entityIndexes) != groupID ||
        foeEcsIndexesGetGroupID(resourceIndexes) != groupID) {
        FOE_LOG(SimulationState, Error,
                "foeGroupData::addDynamicGroup - ID Groups don't match between the indexes and "
                "importer");
        return false;
    }

    // Must be within the dynamic groups valid range
    auto groupValue = foeIdGroupToValue(foeEcsIndexesGetGroupID(entityIndexes));
    if (groupValue >= foeIdNumDynamicGroups) {
        FOE_LOG(SimulationState, Error,
                "foeGroupData::addDynamicGroup - ID Group is not within the valid dynamic group "
                "value range");
        return false;
    }

    // Check that the group isn't already used
    if (mDynamicGroups[groupValue].entityIndexes != FOE_NULL_HANDLE) {
        FOE_LOG(SimulationState, Error,
                "foeGroupData::addDynamicGroup - Attempted to add ID group that is already used")
        return false;
    }

    // Check against duplicate names
    if (strcmp(pGroupName, cPersistentName.data()) == 0 ||
        strcmp(pGroupName, cTemporaryName.data()) == 0) {
        FOE_LOG(SimulationState, Error,
                "foeGroupData::addDynamicGroup - Importer group name is a reserved name, either "
                "'Persistent' or 'Temporary'");
        return false;
    }

    for (auto const &it : mDynamicGroups) {
        if (it.pImporter == nullptr)
            continue;

        char const *pDynamicGroupName;
        result = it.pImporter->name(&pDynamicGroupName);
        if (result.value != FOE_SUCCESS) {
            continue;
        }

        if (strcmp(pDynamicGroupName, pGroupName) == 0) {
            FOE_LOG(SimulationState, Error,
                    "foeGroupData::addDynamicGroup - Importer group name already exists");
            return false;
        }
    }

    mDynamicGroups[groupValue].entityIndexes = entityIndexes;
    mDynamicGroups[groupValue].resourceIndexes = resourceIndexes;
    mDynamicGroups[groupValue].pImporter = std::move(pImporter);
    return true;
}

bool foeGroupData::setPersistentImporter(std::unique_ptr<foeImporterBase> &&pImporter) {
    if (pImporter == nullptr) {
        FOE_LOG(SimulationState, Error,
                "foeGroupData::setPersistentImporter - Importer group not given (nullptr)");
        return false;
    }

    foeIdGroup group;
    foeResult result = pImporter->group(&group);

    if (group != foeIdPersistentGroup) {
        FOE_LOG(
            SimulationState, Error,
            "foeGroupData::setPersistentImporter - Importer group given not foeIdPersistentGroup");
        return false;
    }

    mPersistentImporter = std::move(pImporter);
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
        if (it.pImporter == nullptr)
            continue;

        char const *pDynamicGroupName;
        foeResult result = it.pImporter->name(&pDynamicGroupName);
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
        if (it.pImporter == nullptr)
            continue;

        char const *pDynamicGroupName;
        foeResult result = it.pImporter->name(&pDynamicGroupName);
        if (result.value != FOE_SUCCESS) {
            continue;
        }

        if (std::string_view{pDynamicGroupName} == groupName) {
            return it.resourceIndexes;
        }
    }

    return nullptr;
}

auto foeGroupData::importer(foeIdGroup group) noexcept -> foeImporterBase * {
    auto idGroup = foeIdGetGroup(group);

    if (idGroup == foeIdPersistentGroup) {
        return persistentImporter();
    } else if (idGroup == foeIdTemporaryGroup) {
        return nullptr;
    }

    return mDynamicGroups[foeIdGroupToValue(idGroup)].pImporter.get();
}

auto foeGroupData::importer(std::string_view groupName) noexcept -> foeImporterBase * {
    char const *pGroupName;

    if (groupName == cTemporaryName) {
        return nullptr;
    } else if (groupName == cPersistentName) {
        return persistentImporter();
    } else if (mPersistentImporter != nullptr) {
        foeResult result = mPersistentImporter->name(&pGroupName);
        if (result.value == FOE_SUCCESS && std::string_view{pGroupName} == groupName) {
            return persistentImporter();
        }
    }

    for (auto const &it : mDynamicGroups) {
        if (it.pImporter == nullptr)
            continue;

        foeResult result = it.pImporter->name(&pGroupName);
        if (result.value != FOE_SUCCESS) {
            continue;
        }

        if (std::string_view{pGroupName} == groupName) {
            return it.pImporter.get();
        }
    }

    return nullptr;
}

foeEcsIndexes foeGroupData::persistentEntityIndexes() noexcept { return mPersistentEntityIndexes; }

foeEcsIndexes foeGroupData::persistentResourceIndexes() noexcept {
    return mPersistentResourceIndexes;
}

auto foeGroupData::persistentImporter() noexcept -> foeImporterBase * {
    return mPersistentImporter.get();
}

auto foeGroupData::temporaryEntityIndexes() noexcept -> foeEcsIndexes {
    return mTemporaryEntityIndexes;
}

auto foeGroupData::temporaryResourceIndexes() noexcept -> foeEcsIndexes {
    return mTemporaryResourceIndexes;
}

foeResourceCreateInfo foeGroupData::getResourceDefinition(foeId id) {
    if (mPersistentImporter != nullptr) {
        foeResourceCreateInfo createInfo = FOE_NULL_HANDLE;
        foeResult result = mPersistentImporter->getResource(id, &createInfo);
        if (result.value == FOE_SUCCESS && createInfo != FOE_NULL_HANDLE) {
            return createInfo;
        }
    }

    for (auto it = mDynamicGroups.rbegin(); it != mDynamicGroups.rend(); ++it) {
        if (it->pImporter != nullptr) {
            foeResourceCreateInfo createInfo = FOE_NULL_HANDLE;
            foeResult result = it->pImporter->getResource(id, &createInfo);
            if (result.value == FOE_SUCCESS && createInfo != FOE_NULL_HANDLE) {
                return createInfo;
            }
        }
    }

    return FOE_NULL_HANDLE;
}

std::filesystem::path foeGroupData::findExternalFile(std::filesystem::path externalFilePath) {
    if (mPersistentImporter != nullptr) {
        uint32_t pathLength;
        foeResult result = mPersistentImporter->findExternalFile(externalFilePath.string().c_str(),
                                                                 &pathLength, NULL);
        if (result.value == FOE_SUCCESS) {
            std::string path;
            do {
                path.resize(pathLength);
                result = mPersistentImporter->findExternalFile(externalFilePath.string().c_str(),
                                                               &pathLength, path.data());
            } while (result.value != FOE_SUCCESS);
            return path;
        }
    }

    for (auto it = mDynamicGroups.rbegin(); it != mDynamicGroups.rend(); ++it) {
        if (it->pImporter != nullptr) {
            uint32_t pathLength;
            foeResult result = it->pImporter->findExternalFile(externalFilePath.string().c_str(),
                                                               &pathLength, NULL);
            if (result.value == FOE_SUCCESS) {
                std::string path;
                do {
                    path.resize(pathLength);
                    result = it->pImporter->findExternalFile(externalFilePath.string().c_str(),
                                                             &pathLength, path.data());
                } while (result.value != FOE_SUCCESS);
                return path;
            }
        }
    }

    return {};
}