/*
    Copyright (C) 2022 George Cave.

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

#include <foe/resource/records.h>

#include "result.h"

#include <algorithm>
#include <mutex>
#include <shared_mutex>
#include <vector>

constexpr size_t cInvalidEditorRecord = 0;

namespace {

struct SavedRecord {
    /// The GroupID thise create info was loaded/imported from
    foeIdGroup sourceGroup;
    foeResourceCreateInfo createInfo;
};

struct ResourceRecordData {
    foeResourceID resourceID;
    std::vector<SavedRecord> savedRecords;
    std::vector<foeResourceCreateInfo> sessionRecords;
    size_t currentSessionRecord;
};

struct ResourceRecords {
    std::shared_mutex sync;
    std::vector<ResourceRecordData> records;
};

FOE_DEFINE_HANDLE_CASTS(resource_records, ResourceRecords, foeResourceRecords)

} // namespace

extern "C" foeResult foeResourceCreateRecords(foeResourceRecords *pResourceRecords) {
    ResourceRecords *pNewRecords = new ResourceRecords;

    *pResourceRecords = resource_records_to_handle(pNewRecords);

    return to_foeResult(FOE_RESOURCE_SUCCESS);
}

extern "C" void foeResourceDestroyRecords(foeResourceRecords resourceRecords) {
    ResourceRecords *pRecords = resource_records_from_handle(resourceRecords);

    // Iterate through all records, decrementing references to stored records
    for (auto const &resource : pRecords->records) {
        // Saved
        for (auto const &record : resource.savedRecords) {
            int count = foeResourceCreateInfoDecrementRefCount(record.createInfo);
            if (count == 0)
                foeDestroyResourceCreateInfo(record.createInfo);
        }

        // Session
        for (auto record : resource.sessionRecords) {
            int count = foeResourceCreateInfoDecrementRefCount(record);
            if (count == 0)
                foeDestroyResourceCreateInfo(record);
        }
    }

    delete pRecords;
}

extern "C" foeResult foeResourceAddRecordEntry(foeResourceRecords resourceRecords,
                                               foeResourceID resourceID) {
    ResourceRecords *pRecords = resource_records_from_handle(resourceRecords);
    std::unique_lock lock{pRecords->sync};

    auto record = std::lower_bound(pRecords->records.begin(), pRecords->records.end(), resourceID,
                                   [](auto const &record, foeResourceID resourceID) {
                                       return record.resourceID < resourceID;
                                   });

    // Resource should not already exist
    if (record != pRecords->records.end() && record->resourceID == resourceID) {
        return to_foeResult(FOE_RESOURCE_ERROR_EXISTING_RECORD);
    }

    pRecords->records.emplace(record, ResourceRecordData{
                                          .resourceID = resourceID,
                                          .currentSessionRecord = cInvalidEditorRecord,
                                      });

    return to_foeResult(FOE_RESOURCE_SUCCESS);
}

extern "C" foeResult foeResourceRemoveRecordEntry(foeResourceRecords resourceRecords,
                                                  foeResourceID resourceID) {
    if (foeIdGetGroup(resourceID) == foeIdPersistentGroup) {
        return to_foeResult(FOE_RESOURCE_ERROR_CANNOT_REMOVE_NON_PERSISTENT_RECORDS);
    }

    ResourceRecords *pRecords = resource_records_from_handle(resourceRecords);
    std::unique_lock lock{pRecords->sync};

    auto record = std::lower_bound(pRecords->records.begin(), pRecords->records.end(), resourceID,
                                   [](auto const &record, foeResourceID resourceID) {
                                       return record.resourceID < resourceID;
                                   });

    if (record != pRecords->records.end() && record->resourceID == resourceID) {
        pRecords->records.erase(record);
    }

    return to_foeResult(FOE_RESOURCE_SUCCESS);
}

extern "C" foeResult foeResourceAddSavedRecord(foeResourceRecords resourceRecords,
                                               foeIdGroup groupID,
                                               foeResourceID resourceID,
                                               foeResourceCreateInfo createInfo) {
    ResourceRecords *pRecords = resource_records_from_handle(resourceRecords);
    std::unique_lock lock{pRecords->sync};

    auto record = std::lower_bound(pRecords->records.begin(), pRecords->records.end(), resourceID,
                                   [](auto const &record, foeResourceID resourceID) {
                                       return record.resourceID < resourceID;
                                   });

    if (record == pRecords->records.end() || record->resourceID != resourceID) {
        return to_foeResult(FOE_RESOURCE_ERROR_NON_EXISTING_RECORD);
    }

    foeResourceCreateInfoIncrementRefCount(createInfo);

    record->savedRecords.emplace_back(SavedRecord{
        .sourceGroup = groupID,
        .createInfo = createInfo,
    });

    return to_foeResult(FOE_RESOURCE_SUCCESS);
}

extern "C" foeResult foeResourceAddSessionRecord(foeResourceRecords resourceRecords,
                                                 foeResourceID resourceID,
                                                 foeResourceCreateInfo createInfo) {
    ResourceRecords *pRecords = resource_records_from_handle(resourceRecords);
    std::unique_lock lock{pRecords->sync};

    auto record = std::lower_bound(pRecords->records.begin(), pRecords->records.end(), resourceID,
                                   [](auto const &record, foeResourceID resourceID) {
                                       return record.resourceID < resourceID;
                                   });

    if (record == pRecords->records.end() || record->resourceID != resourceID) {
        return to_foeResult(FOE_RESOURCE_ERROR_NON_EXISTING_RECORD);
    }

    foeResourceCreateInfoIncrementRefCount(createInfo);

    for (auto i = record->currentSessionRecord; i < record->sessionRecords.size(); ++i) {
        int count = foeResourceCreateInfoDecrementRefCount(record->sessionRecords[i]);
        if (count == 0)
            foeDestroyResourceCreateInfo(record->sessionRecords[i]);
    }

    record->sessionRecords.erase(record->sessionRecords.begin() + record->currentSessionRecord,
                                 record->sessionRecords.end());

    record->sessionRecords.emplace_back(createInfo);

    return to_foeResult(FOE_RESOURCE_SUCCESS);
}

extern "C" foeResult foeResourceUndoSessionRecord(foeResourceRecords resourceRecords,
                                                  foeResourceID resourceID) {
    ResourceRecords *pRecords = resource_records_from_handle(resourceRecords);
    std::unique_lock lock{pRecords->sync};

    auto record = std::lower_bound(pRecords->records.begin(), pRecords->records.end(), resourceID,
                                   [](auto const &record, foeResourceID resourceID) {
                                       return record.resourceID < resourceID;
                                   });

    if (record == pRecords->records.end() || record->resourceID != resourceID) {
        return to_foeResult(FOE_RESOURCE_ERROR_NON_EXISTING_RECORD);
    }

    if (record->currentSessionRecord == 0)
        return to_foeResult(FOE_RESOURCE_CANNOT_UNDO);

    --record->currentSessionRecord;

    return to_foeResult(FOE_RESOURCE_SUCCESS);
}

extern "C" foeResult foeResourceRedoSessionRecord(foeResourceRecords resourceRecords,
                                                  foeResourceID resourceID) {
    ResourceRecords *pRecords = resource_records_from_handle(resourceRecords);
    std::unique_lock lock{pRecords->sync};

    auto record = std::lower_bound(pRecords->records.begin(), pRecords->records.end(), resourceID,
                                   [](auto const &record, foeResourceID resourceID) {
                                       return record.resourceID < resourceID;
                                   });

    if (record == pRecords->records.end() || record->resourceID != resourceID) {
        return to_foeResult(FOE_RESOURCE_ERROR_NON_EXISTING_RECORD);
    }

    if (record->sessionRecords.size() <= record->currentSessionRecord)
        return to_foeResult(FOE_RESOURCE_CANNOT_REDO);

    ++record->currentSessionRecord;
    return to_foeResult(FOE_RESOURCE_SUCCESS);
}

extern "C" foeResult foeResourceRecordsGetCreateInfo(foeResourceRecords resourceRecords,
                                                     foeResourceID resourceID,
                                                     foeResourceCreateInfo *pCreateInfo) {
    ResourceRecords *pRecords = resource_records_from_handle(resourceRecords);
    std::shared_lock lock{pRecords->sync};

    auto record = std::lower_bound(pRecords->records.begin(), pRecords->records.end(), resourceID,
                                   [](auto const &record, foeResourceID resourceID) {
                                       return record.resourceID < resourceID;
                                   });

    if (record == pRecords->records.end() || record->resourceID != resourceID) {
        return to_foeResult(FOE_RESOURCE_ERROR_NON_EXISTING_RECORD);
    }

    if (record->currentSessionRecord == 0) {
        if (record->savedRecords.empty()) {
            to_foeResult(FOE_RESOURCE_ERROR_NO_RECORDS);
        }

        *pCreateInfo = (record->savedRecords.end() - 1)->createInfo;
    } else {
        *pCreateInfo = record->sessionRecords[record->currentSessionRecord - 1];
    }

    // @TODO Re-add auto-increment
    // foeResourceCreateInfoIncrementRefCount(*pCreateInfo);
    return to_foeResult(FOE_RESOURCE_SUCCESS);
}

extern "C" foeResult foeResourceRecordsGetModifiedHistory(foeResourceRecords resourceRecords,
                                                          foeResourceID resourceID,
                                                          foeResourceCreateInfo *pCreateInfo) {
    ResourceRecords *pRecords = resource_records_from_handle(resourceRecords);
    std::shared_lock lock{pRecords->sync};

    auto record = std::lower_bound(pRecords->records.begin(), pRecords->records.end(), resourceID,
                                   [](auto const &record, foeResourceID resourceID) {
                                       return record.resourceID < resourceID;
                                   });

    if (record == pRecords->records.end() || record->resourceID != resourceID) {
        return to_foeResult(FOE_RESOURCE_ERROR_NON_EXISTING_RECORD);
    }

    if (record->currentSessionRecord == 0)
        return to_foeResult(FOE_RESOURCE_NO_MODIFIED_RECORD);

    *pCreateInfo = record->sessionRecords[record->currentSessionRecord - 1];
    foeResourceCreateInfoIncrementRefCount(*pCreateInfo);
    return to_foeResult(FOE_RESOURCE_SUCCESS);
}

extern "C" void foeResourceRecordsUpdatedSavedRecords(foeResourceRecords resourceRecords) {
    ResourceRecords *pRecords = resource_records_from_handle(resourceRecords);
    std::unique_lock lock{pRecords->sync};

    for (auto &resource : pRecords->records) {
        // If there is no differen session record, nothing to do, it's still the last saved record
        if (resource.currentSessionRecord == 0)
            continue;

        foeResourceCreateInfo createInfo =
            resource.sessionRecords[resource.currentSessionRecord - 1];

        // No matter what, this entry is about to be used
        foeResourceCreateInfoIncrementRefCount(createInfo);

        if (!resource.savedRecords.empty()) {
            auto lastSaved = resource.savedRecords.end() - 1;
            if (lastSaved->sourceGroup == foeIdPersistentGroup) {
                int count = foeResourceCreateInfoDecrementRefCount(lastSaved->createInfo);
                if (count == 0)
                    foeDestroyResourceCreateInfo(lastSaved->createInfo);

                resource.savedRecords.erase(lastSaved);
            }
        }

        resource.savedRecords.emplace_back(SavedRecord{
            .sourceGroup = foeIdPersistentGroup,
            .createInfo = createInfo,
        });
    }
}