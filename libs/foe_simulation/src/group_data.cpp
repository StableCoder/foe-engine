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

#include <foe/simulation/group_data.hpp>

#include "log.hpp"

bool foeGroupData::addDynamicGroup(std::unique_ptr<foeIdIndexGenerator> &&pEntityIndices,
                                   std::unique_ptr<foeIdIndexGenerator> &&pResourceIndices,
                                   std::unique_ptr<foeImporterBase> &&pImporter) {
    // Make sure both items are valid pointers
    if (pEntityIndices == nullptr || pResourceIndices == nullptr || pImporter == nullptr) {
        FOE_LOG(SimulationState, Error,
                "foeGroupData::addDynamicGroup - Either the given indices or importer are nullptr");
        return false;
    }

    // Check against blank name for the importer
    if (pImporter->name().empty()) {
        FOE_LOG(SimulationState, Error,
                "foeGroupData::addDynamicGroup - Importer had a blank group name");
        return false;
    }

    // Check both have the same ID Group
    if (pEntityIndices->groupID() != pImporter->group() ||
        pResourceIndices->groupID() != pImporter->group()) {
        FOE_LOG(SimulationState, Error,
                "foeGroupData::addDynamicGroup - ID Groups don't match between the indices and "
                "importer");
        return false;
    }

    // Must be within the dynamic groups valid range
    auto groupValue = foeIdGroupToValue(pEntityIndices->groupID());
    if (groupValue >= foeIdNumDynamicGroups) {
        FOE_LOG(SimulationState, Error,
                "foeGroupData::addDynamicGroup - ID Group is not within the valid dynamic group "
                "value range");
        return false;
    }

    // Check that the group isn't already used
    if (mDynamicGroups[groupValue].pEntityIndices != nullptr) {
        FOE_LOG(SimulationState, Error,
                "foeGroupData::addDynamicGroup - Attempted to add ID group that is already used")
        return false;
    }

    // Check against duplicate names
    if (pImporter->name() == cPersistentName || pImporter->name() == cTemporaryName) {
        FOE_LOG(SimulationState, Error,
                "foeGroupData::addDynamicGroup - Importer group name is a reserved name, either "
                "'Persistent' or 'Temporary'");
        return false;
    }

    for (auto const &it : mDynamicGroups) {
        if (it.pImporter != nullptr && it.pImporter->name() == pImporter->name()) {
            FOE_LOG(SimulationState, Error,
                    "foeGroupData::addDynamicGroup - Importer group name already exists");
            return false;
        }
    }

    mDynamicGroups[groupValue].pEntityIndices = std::move(pEntityIndices);
    mDynamicGroups[groupValue].pResourceIndices = std::move(pResourceIndices);
    mDynamicGroups[groupValue].pImporter = std::move(pImporter);
    return true;
}

bool foeGroupData::setPersistentImporter(std::unique_ptr<foeImporterBase> &&pImporter) {
    if (pImporter == nullptr) {
        FOE_LOG(SimulationState, Error,
                "foeGroupData::setPersistentImporter - Importer group not given (nullptr)");
        return false;
    }

    if (pImporter->group() != foeIdPersistentGroup) {
        FOE_LOG(
            SimulationState, Error,
            "foeGroupData::setPersistentImporter - Importer group given not foeIdPersistentGroup");
        return false;
    }

    mPersistentImporter = std::move(pImporter);
    return true;
}

auto foeGroupData::entityIndices(foeIdGroup group) noexcept -> foeIdIndexGenerator * {
    auto idGroup = foeIdGetGroup(group);

    if (idGroup == foeIdPersistentGroup) {
        return persistentEntityIndices();
    } else if (idGroup == foeIdTemporaryGroup) {
        return temporaryEntityIndices();
    }

    return mDynamicGroups[foeIdGroupToValue(idGroup)].pEntityIndices.get();
}

auto foeGroupData::entityIndices(std::string_view groupName) noexcept -> foeIdIndexGenerator * {
    if (groupName == cPersistentName) {
        return persistentEntityIndices();
    } else if (groupName == cTemporaryName) {
        return temporaryEntityIndices();
    }

    for (auto const &it : mDynamicGroups) {
        if (it.pImporter != nullptr && it.pImporter->name() == groupName) {
            return it.pEntityIndices.get();
        }
    }

    return nullptr;
}

auto foeGroupData::resourceIndices(foeIdGroup group) noexcept -> foeIdIndexGenerator * {
    auto idGroup = foeIdGetGroup(group);

    if (idGroup == foeIdPersistentGroup) {
        return persistentResourceIndices();
    } else if (idGroup == foeIdTemporaryGroup) {
        return temporaryResourceIndices();
    }

    return mDynamicGroups[foeIdGroupToValue(idGroup)].pResourceIndices.get();
}

auto foeGroupData::resourceIndices(std::string_view groupName) noexcept -> foeIdIndexGenerator * {
    if (groupName == cPersistentName) {
        return persistentResourceIndices();
    } else if (groupName == cTemporaryName) {
        return temporaryResourceIndices();
    }

    for (auto const &it : mDynamicGroups) {
        if (it.pImporter != nullptr && it.pImporter->name() == groupName) {
            return it.pResourceIndices.get();
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
    if (groupName == cPersistentName ||
        (mPersistentImporter != nullptr && mPersistentImporter->name() == groupName)) {
        return persistentImporter();
    } else if (groupName == cTemporaryName) {
        return nullptr;
    }

    for (auto const &it : mDynamicGroups) {
        if (it.pImporter != nullptr && it.pImporter->name() == groupName) {
            return it.pImporter.get();
        }
    }

    return nullptr;
}

auto foeGroupData::persistentEntityIndices() noexcept -> foeIdIndexGenerator * {
    return &mPersistentEntityIndices;
}

auto foeGroupData::persistentResourceIndices() noexcept -> foeIdIndexGenerator * {
    return &mPersistentResourceIndices;
}

auto foeGroupData::persistentImporter() noexcept -> foeImporterBase * {
    return mPersistentImporter.get();
}

auto foeGroupData::temporaryEntityIndices() noexcept -> foeIdIndexGenerator * {
    return &mTemporaryEntityIndices;
}

auto foeGroupData::temporaryResourceIndices() noexcept -> foeIdIndexGenerator * {
    return &mTemporaryResourceIndices;
}

foeResourceCreateInfoBase *foeGroupData::getResourceDefinition(foeId id) {
    if (mPersistentImporter != nullptr) {
        if (auto *pCreateInfo = mPersistentImporter->getResource(id); pCreateInfo) {
            return pCreateInfo;
        }
    }

    for (auto it = mDynamicGroups.rbegin(); it != mDynamicGroups.rend(); ++it) {
        if (it->pImporter != nullptr) {
            if (auto *pCreateInfo = it->pImporter->getResource(id)) {
                return pCreateInfo;
            }
        }
    }

    return nullptr;
}

std::filesystem::path foeGroupData::findExternalFile(std::filesystem::path externalFilePath) {
    std::filesystem::path foundPath;
    if (mPersistentImporter != nullptr) {
        foundPath = mPersistentImporter->findExternalFile(externalFilePath);
        if (!foundPath.empty())
            return foundPath;
    }

    for (auto it = mDynamicGroups.rbegin(); it != mDynamicGroups.rend(); ++it) {
        if (it->pImporter != nullptr) {
            foundPath = it->pImporter->findExternalFile(externalFilePath);
            if (!foundPath.empty())
                return foundPath;
        }
    }

    return foundPath;
}