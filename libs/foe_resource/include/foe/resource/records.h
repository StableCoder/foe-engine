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

#ifndef FOE_RESOURCE_RECORDS_H
#define FOE_RESOURCE_RECORDS_H

#include <foe/ecs/id.h>
#include <foe/error_code.h>
#include <foe/handle.h>
#include <foe/resource/create_info.h>
#include <foe/resource/export.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeResourceRecords)

FOE_RES_EXPORT foeResult foeResourceCreateRecords(foeResourceRecords *pResourceRecords);

FOE_RES_EXPORT void foeResourceDestroyRecords(foeResourceRecords resourceRecords);

FOE_RES_EXPORT foeResult foeResourceAddRecordEntry(foeResourceRecords resourceRecords,
                                                   foeResourceID resourceID);

// Only should work on persistent resources
FOE_RES_EXPORT foeResult foeResourceRemoveRecordEntry(foeResourceRecords resourceRecords,
                                                      foeResourceID resourceID);

// Saved
FOE_RES_EXPORT foeResult foeResourceAddSavedRecord(foeResourceRecords resourceRecords,
                                                   foeIdGroup groupID,
                                                   foeResourceID resourceID,
                                                   foeResourceCreateInfo createInfo);

// Session
FOE_RES_EXPORT foeResult foeResourceAddSessionRecord(foeResourceRecords resourceRecords,
                                                     foeResourceID resourceID,
                                                     foeResourceCreateInfo createInfo);

FOE_RES_EXPORT foeResult foeResourceUndoSessionRecord(foeResourceRecords resourceRecords,
                                                      foeResourceID resourceID);

FOE_RES_EXPORT foeResult foeResourceRedoSessionRecord(foeResourceRecords resourceRecords,
                                                      foeResourceID resourceID);

FOE_RES_EXPORT foeResult foeResourceRecordsGetCreateInfo(foeResourceRecords resourceRecords,
                                                         foeResourceID resourceID,
                                                         foeResourceCreateInfo *pCreateInfo);

// Only returns if the create info differs from the last saved history entry
FOE_RES_EXPORT foeResult foeResourceRecordsGetModifiedHistory(foeResourceRecords resourceRecords,
                                                              foeResourceID resourceID,
                                                              foeResourceCreateInfo *pCreateInfo);

FOE_RES_EXPORT void foeResourceRecordsUpdatedSavedRecords(foeResourceRecords resourceRecords);

#ifdef __cplusplus
}
#endif

#endif // FOE_RESOURCE_RECORDS_H