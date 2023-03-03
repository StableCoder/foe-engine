// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_RESOURCE_TYPE_DEFS_H
#define FOE_RESOURCE_TYPE_DEFS_H

typedef int foeResourceType;

typedef struct foeResourceBase {
    foeResourceType rType;
    void *pNext;
} foeResourceBase;

#endif // FOE_RESOURCE_TYPE_DEFS_H