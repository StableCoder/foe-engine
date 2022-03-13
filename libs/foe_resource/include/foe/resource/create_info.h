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

FOE_RES_EXPORT foeErrorCode
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