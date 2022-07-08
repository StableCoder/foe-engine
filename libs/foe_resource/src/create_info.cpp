// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/resource/create_info.h>

#include <atomic>

#include "log.hpp"
#include "result.h"

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

extern "C" foeResult foeCreateResourceCreateInfo(foeResourceCreateInfoType type,
                                                 void (*pDestroyFn)(foeResourceCreateInfoType type,
                                                                    void *),
                                                 size_t size,
                                                 void *pData,
                                                 void (*pDataFn)(void *, void *),
                                                 foeResourceCreateInfo *pCreateInfo) {
    if (pDataFn == nullptr)
        return to_foeResult(FOE_RESOURCE_ERROR_DATA_FUNCTION_NOT_PROVIDED);

    foeResourceCreateInfoImpl *pNewCI =
        (foeResourceCreateInfoImpl *)malloc(sizeof(foeResourceCreateInfoImpl) + size);
    if (pNewCI == NULL)
        return to_foeResult(FOE_RESOURCE_ERROR_OUT_OF_MEMORY);

    new (pNewCI) foeResourceCreateInfoImpl(type, pDestroyFn);

    pDataFn(pData, (void *)foeResourceCreateInfoGetData(resource_create_info_to_handle(pNewCI)));

    *pCreateInfo = resource_create_info_to_handle(pNewCI);

    FOE_LOG(foeResourceCore, Verbose, "[{},{}] foeResourceCreateInfo - Created", (void *)pNewCI,
            type)

    return to_foeResult(FOE_RESOURCE_SUCCESS);
}

extern "C" void foeDestroyResourceCreateInfo(foeResourceCreateInfo createInfo) {
    auto *pCreateInfo = resource_create_info_from_handle(createInfo);

    FOE_LOG(foeResourceCore, Verbose, "[{},{}] foeResourceCreateInfo - Destroying",
            (void *)pCreateInfo, pCreateInfo->type)

    int refCount = pCreateInfo->refCount;
    if (refCount != 0) {
        FOE_LOG(foeResourceCore, Warning,
                "[{},{}] foeResourceCreateInfo - Destroying with a non-zero reference count of: {}",
                (void *)pCreateInfo, pCreateInfo->type, refCount)
    }

    if (pCreateInfo->pDestroyFn != nullptr) {
        pCreateInfo->pDestroyFn(pCreateInfo->type,
                                (void *)foeResourceCreateInfoGetData(createInfo));
    }

    FOE_LOG(foeResourceCore, Verbose, "[{},{}] foeResourceCreateInfo - Destroyed",
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

extern "C" void const *foeResourceCreateInfoGetData(foeResourceCreateInfo createInfo) {
    auto *pCreateInfo = resource_create_info_from_handle(createInfo);
    return (char *)pCreateInfo + sizeof(foeResourceCreateInfoImpl);
}