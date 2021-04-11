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

#include "group_data.hpp"

#include <foe/log.hpp>

bool foeGroupData::addDynamicGroup(std::unique_ptr<foeIdIndexGenerator> &&pIndices,
                                   std::unique_ptr<foeImporterBase> &&pImporter) {
    // Make sure both items are valid pointers
    if (pIndices == nullptr || pImporter == nullptr) {
        FOE_LOG(General, Error,
                "foeGroupData::addDynamicGroup - Either the given indices or importer are nullptr");
        return false;
    }

    // Check against blank name for the importer
    if (pImporter->name().empty()) {
        FOE_LOG(General, Error, "foeGroupData::addDynamicGroup - Importer had a blank group name");
        return false;
    }

    // Check both have the same ID Group
    if (pIndices->groupID() != pImporter->group()) {
        FOE_LOG(General, Error,
                "foeGroupData::addDynamicGroup - ID Groups don't match between the indices and "
                "importer");
        return false;
    }

    // Must be within the dynamic groups valid range
    auto groupValue = foeIdGroupToValue(pIndices->groupID());
    if (groupValue >= foeIdMaxDynamicGroups) {
        FOE_LOG(General, Error,
                "foeGroupData::addDynamicGroup - ID Group is not within the valid dynamic group "
                "value range");
        return false;
    }

    // Check that the group isn't already used
    if (mDynamicGroups[groupValue].pIndices != nullptr) {
        FOE_LOG(General, Error,
                "foeGroupData::addDynamicGroup - Attempted to add ID group that is already used")
        return false;
    }

    // Check against duplicate names
    if (pImporter->name() == cPersistentName || pImporter->name() == cTemporaryName) {
        FOE_LOG(General, Error,
                "foeGroupData::addDynamicGroup - Importer group name is a reserved name, either "
                "'Persistent' or 'Temporary'");
        return false;
    }

    for (auto const &it : mDynamicGroups) {
        if (it.pImporter != nullptr && it.pImporter->name() == pImporter->name()) {
            FOE_LOG(General, Error,
                    "foeGroupData::addDynamicGroup - Importer group name already exists");
            return false;
        }
    }

    mDynamicGroups[groupValue].pIndices = std::move(pIndices);
    mDynamicGroups[groupValue].pImporter = std::move(pImporter);
    return true;
}

bool foeGroupData::setPersistentImporter(std::unique_ptr<foeImporterBase> &&pImporter) {
    if (pImporter == nullptr) {
        FOE_LOG(General, Error,
                "foeGroupData::setPersistentImporter - Importer group not given (nullptr)");
        return false;
    }

    if (pImporter->group() != foeIdPersistentGroup) {
        FOE_LOG(
            General, Error,
            "foeGroupData::setPersistentImporter - Importer group given not foeIdPersistentGroup");
        return false;
    }

    mPersistentImporter = std::move(pImporter);
    return true;
}

auto foeGroupData::indices(foeIdGroup group) noexcept -> foeIdIndexGenerator * {
    auto idGroup = foeIdGetGroup(group);

    if (idGroup == foeIdPersistentGroup) {
        return persistentIndices();
    } else if (idGroup == foeIdTemporaryGroup) {
        return temporaryIndices();
    }

    return mDynamicGroups[foeIdGroupToValue(idGroup)].pIndices.get();
}

auto foeGroupData::indices(std::string_view groupName) noexcept -> foeIdIndexGenerator * {
    if (groupName == cPersistentName) {
        return persistentIndices();
    } else if (groupName == cTemporaryName) {
        return temporaryIndices();
    }

    for (auto const &it : mDynamicGroups) {
        if (it.pImporter != nullptr && it.pImporter->name() == groupName) {
            return it.pIndices.get();
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

auto foeGroupData::persistentIndices() noexcept -> foeIdIndexGenerator * {
    return &mPersistentIndices;
}
auto foeGroupData::persistentImporter() noexcept -> foeImporterBase * {
    return mPersistentImporter.get();
}

auto foeGroupData::temporaryIndices() noexcept -> foeIdIndexGenerator * {
    return &mTemporaryIndices;
}

bool foeGroupData::getResourceDefinition(foeId id, foeResourceCreateInfoBase **ppCreateInfo) {
    if (mPersistentImporter != nullptr && mPersistentImporter->getResource(id, ppCreateInfo))
        return true;

    for (auto it = mDynamicGroups.rbegin(); it != mDynamicGroups.rend(); ++it) {
        if (it->pImporter != nullptr && it->pImporter->getResource(id, ppCreateInfo))
            return true;
    }

    return false;
}