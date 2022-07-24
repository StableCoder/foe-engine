// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_RESOURCE_CREATE_INFO_H
#define FOE_RESOURCE_CREATE_INFO_H

#include <foe/error_code.h>
#include <foe/handle.h>
#include <foe/resource/export.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeResourceCreateInfo)

typedef int foeResourceCreateInfoType;

FOE_RES_EXPORT foeResultSet
foeCreateResourceCreateInfo(foeResourceCreateInfoType type,
                            void (*pDestroyFn)(foeResourceCreateInfoType, void *),
                            size_t size,
                            void *pData,
                            void (*pDataFn)(void *, void *),
                            foeResourceCreateInfo *pCreateInfo);

FOE_RES_EXPORT void foeDestroyResourceCreateInfo(foeResourceCreateInfo createInfo);

FOE_RES_EXPORT foeResourceCreateInfoType
foeResourceCreateInfoGetType(foeResourceCreateInfo createInfo);

FOE_RES_EXPORT int foeResourceCreateInfoGetRefCount(foeResourceCreateInfo createInfo);
FOE_RES_EXPORT int foeResourceCreateInfoIncrementRefCount(foeResourceCreateInfo createInfo);
FOE_RES_EXPORT int foeResourceCreateInfoDecrementRefCount(foeResourceCreateInfo createInfo);

FOE_RES_EXPORT void const *foeResourceCreateInfoGetData(foeResourceCreateInfo createInfo);

#ifdef __cplusplus
}
#endif

#endif // FOE_RESOURCE_CREATE_INFO_H