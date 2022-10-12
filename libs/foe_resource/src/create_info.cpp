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
    std::atomic_int refCount;
    PFN_foeResourceCreateInfoCleanup cleanupFn;

    foeResourceCreateInfoImpl(foeResourceCreateInfoType type,
                              PFN_foeResourceCreateInfoCleanup cleanupFn) :
        type{type}, refCount{1}, cleanupFn{cleanupFn} {}
};

FOE_DEFINE_HANDLE_CASTS(resource_create_info, foeResourceCreateInfoImpl, foeResourceCreateInfo)

} // namespace

extern "C" foeResultSet foeCreateResourceCreateInfo(foeResourceCreateInfoType type,
                                                    PFN_foeResourceCreateInfoCleanup cleanupFn,
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

    new (pNewCI) foeResourceCreateInfoImpl(type, cleanupFn);

    pDataFn(pData, (void *)foeResourceCreateInfoGetData(resource_create_info_to_handle(pNewCI)));

    *pCreateInfo = resource_create_info_to_handle(pNewCI);

    FOE_LOG(foeResourceCore, FOE_LOG_LEVEL_VERBOSE, "[{},{}] foeResourceCreateInfo - Created",
            (void *)pNewCI, type)

    return to_foeResult(FOE_RESOURCE_SUCCESS);
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
    int refCount = --pCreateInfo->refCount;

    if (refCount == 0) {
        FOE_LOG(foeResourceCore, FOE_LOG_LEVEL_VERBOSE,
                "[{},{}] foeResourceCreateInfo - Destroying", (void *)pCreateInfo,
                pCreateInfo->type)

        if (pCreateInfo->cleanupFn != nullptr) {
            pCreateInfo->cleanupFn((void *)foeResourceCreateInfoGetData(createInfo));
        }

        FOE_LOG(foeResourceCore, FOE_LOG_LEVEL_VERBOSE, "[{},{}] foeResourceCreateInfo - Destroyed",
                (void *)pCreateInfo, pCreateInfo->type)

        pCreateInfo->~foeResourceCreateInfoImpl();

        free(pCreateInfo);
    }

    return refCount;
}

extern "C" void const *foeResourceCreateInfoGetData(foeResourceCreateInfo createInfo) {
    auto *pCreateInfo = resource_create_info_from_handle(createInfo);
    return (char *)pCreateInfo + sizeof(foeResourceCreateInfoImpl);
}