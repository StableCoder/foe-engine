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

#include <foe/resource/create_info.h>

#include <atomic>

#include "error_code.hpp"
#include "log.hpp"

namespace {

struct foeResourceCreateInfoImpl {
    foeResourceCreateInfoType type;
    std::atomic_int refCount{0};
    void (*pDestroyFn)(foeResourceCreateInfoType, void *);

    foeResourceCreateInfoImpl(foeResourceCreateInfoType type,
                              void (*pDestroyFn)(foeResourceCreateInfoType, void *)) :
        type{type}, pDestroyFn{pDestroyFn} {}
};

FOE_DEFINE_HANDLE_CASTS(resource_create_info, foeResourceCreateInfoImpl, foeResourceCreateInfo)

} // namespace

extern "C" foeErrorCode foeCreateResourceCreateInfo(
    foeResourceCreateInfoType type,
    void (*pDestroyFn)(foeResourceCreateInfoType type, void *),
    size_t size,
    foeResourceCreateInfo *pCreateInfo) {
    auto *pNewCI = (foeResourceCreateInfoImpl *)malloc(sizeof(foeResourceCreateInfoImpl) + size);
    if (pNewCI == nullptr) {
        return foeToErrorCode(FOE_RESOURCE_ERROR_OUT_OF_HOST_MEMORY);
    }

    new (pNewCI) foeResourceCreateInfoImpl(type, pDestroyFn);

    *pCreateInfo = resource_create_info_to_handle(pNewCI);

    FOE_LOG(foeResourceCore, Verbose, "foeResourceCreateInfo[{},{}] - Created ", (void *)pNewCI,
            type)

    return foeToErrorCode(FOE_RESOURCE_SUCCESS);
}

extern "C" void foeDestroyResourceCreateInfo(foeResourceCreateInfo createInfo) {
    auto *pCreateInfo = resource_create_info_from_handle(createInfo);

    if (pCreateInfo->refCount > 0) {
        FOE_LOG(
            foeResourceCore, Warning,
            "foeResourceCreateInfo[{},{}] - Destroying while there are more than zero references",
            (void *)pCreateInfo, pCreateInfo->type)
    }

    if (pCreateInfo->pDestroyFn != nullptr) {
        pCreateInfo->pDestroyFn(pCreateInfo->type, foeResourceCreateInfoGetData(createInfo));
    }

    FOE_LOG(foeResourceCore, Verbose, "foeResourceCreateInfo[{},{}] - Destroyed",
            (void *)pCreateInfo, pCreateInfo->type)

    pCreateInfo->~foeResourceCreateInfoImpl();

    free(pCreateInfo);
}

extern "C" foeResourceCreateInfoType foeResourceCreateInfoGetType(
    foeResourceCreateInfo createInfo) {
    auto *pCreateInfo = resource_create_info_from_handle(createInfo);
    return pCreateInfo->type;
}

extern "C" int foeResourceCreateInfoGetRefCount(foeResourceCreateInfo createInfo) {
    auto *pCreateInfo = resource_create_info_from_handle(createInfo);
    return pCreateInfo->refCount;
}

extern "C" int foeResourceCreateInfoIncrementRefCount(foeResourceCreateInfo createInfo) {
    auto *pCreateInfo = resource_create_info_from_handle(createInfo);
    return ++pCreateInfo->refCount;
}

extern "C" int foeResourceCreateInfoDecrementRefCount(foeResourceCreateInfo createInfo) {
    auto *pCreateInfo = resource_create_info_from_handle(createInfo);
    return --pCreateInfo->refCount;
}

extern "C" void *foeResourceCreateInfoGetData(foeResourceCreateInfo createInfo) {
    auto *pCreateInfo = resource_create_info_from_handle(createInfo);
    return (char *)pCreateInfo + sizeof(foeResourceCreateInfoImpl);
}