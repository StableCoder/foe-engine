// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_RESOURCE_TYPE_DEFS_H
#define FOE_RESOURCE_TYPE_DEFS_H

#define FOE_RESOURCE_LIBRARY_ID 1000100000

typedef int foeResourceType;

typedef struct foeResourceBase {
    foeResourceType rType;
    void *pNext;
} foeResourceBase;

typedef enum foeResourceResourceType {
    FOE_RESOURCE_RESOURCE_TYPE_UNDEFINED = 1000100000,
    FOE_RESOURCE_RESOURCE_TYPE_REPLACED = 1000100001,
} foeResourceResourceType;

#endif // FOE_RESOURCE_TYPE_DEFS_H