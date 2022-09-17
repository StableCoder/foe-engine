// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>

#include <foe/split_thread_pool.h>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        foeSplitThreadResultToString(X, resultString);                                             \
        CHECK(std::string_view{resultString} == #X);                                               \
    }

TEST_CASE("foeSplitThreadResult - Ensure error codes return correct values and strings") {
    char resultString[FOE_MAX_RESULT_STRING_SIZE];

    SECTION("Generic non-existant negative value") {
        foeSplitThreadResultToString((foeSplitThreadResult)FOE_RESULT_MIN_ENUM, resultString);
        CHECK(std::string_view{resultString} == "FOE_THREAD_POOL_UNKNOWN_ERROR_2147483647");
    }
    SECTION("Generic non-existant positive value") {
        foeSplitThreadResultToString((foeSplitThreadResult)FOE_RESULT_MAX_ENUM, resultString);
        CHECK(std::string_view{resultString} == "FOE_THREAD_POOL_UNKNOWN_SUCCESS_2147483647");
    }

    ERROR_CODE_CATCH_CHECK(FOE_THREAD_POOL_SUCCESS)
    ERROR_CODE_CATCH_CHECK(FOE_THREAD_POOL_ERROR_OUT_OF_MEMORY)
    ERROR_CODE_CATCH_CHECK(FOE_THREAD_POOL_ERROR_ZERO_SYNC_THREADS)
    ERROR_CODE_CATCH_CHECK(FOE_THREAD_POOL_ERROR_ZERO_ASYNC_THREADS)
    ERROR_CODE_CATCH_CHECK(FOE_THREAD_POOL_ERROR_ALLOCATION_FAILED)
    ERROR_CODE_CATCH_CHECK(FOE_THREAD_POOL_ERROR_ALREADY_STARTED)
    ERROR_CODE_CATCH_CHECK(FOE_THREAD_POOL_ERROR_NOT_STARTED)
}