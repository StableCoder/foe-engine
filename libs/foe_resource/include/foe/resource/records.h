// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

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