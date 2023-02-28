// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/resource/resource.h>
#include <foe/resource/resource_fns.h>
#include <foe/resource/result.h>

#include "test_log_sink.hpp"

#include <atomic>

static std::atomic_int gGlobalIterator = 0;
constexpr foeResourceCreateInfoType cCreateInfoType = 1024;
constexpr foeResourceType cResourceType = 2048;

struct TestData {
    int value;
};

foeResourceCreateInfo genCreateInfoFn(void *, foeResourceID) {
    foeResourceCreateInfo createInfo{FOE_NULL_HANDLE};

    auto dataFn = [](void *, void *pDst) {
        auto *pData = (TestData *)pDst;
        pData->value = gGlobalIterator++;
    };

    foeResultSet result = foeCreateResourceCreateInfo(cCreateInfoType, nullptr, sizeof(TestData),
                                                      nullptr, dataFn, &createInfo);
    REQUIRE(result.value == FOE_RESOURCE_SUCCESS);
    REQUIRE(createInfo != FOE_NULL_HANDLE);
    REQUIRE(foeResourceCreateInfoGetRefCount(createInfo) == 1);

    return createInfo;
}
