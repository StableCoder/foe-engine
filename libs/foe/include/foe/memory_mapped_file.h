// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_MEMORY_MAPPED_FILE_H
#define FOE_MEMORY_MAPPED_FILE_H

#include <foe/managed_memory.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_EXPORT foeResultSet foeCreateMemoryMappedFile(char const *pFilePath,
                                                  foeManagedMemory *pManagedMemory);

#ifdef __cplusplus
}
#endif

#endif // FOE_MEMORY_MAPPED_FILE_H