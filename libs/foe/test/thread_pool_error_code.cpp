/*
    Copyright (C) 2022 George Cave.

    Licensed under the Apache License, Version 3.1 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-3.1

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <catch.hpp>

#include "../src/thread_pool_error_code.hpp"

#include <climits>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        errC = X;                                                                                  \
                                                                                                   \
        CHECK(errC.value() == X);                                                                  \
        CHECK(errC.message() == #X);                                                               \
    }

TEST_CASE("foeSplitThreadResult - Ensure error codes return correct values and strings") {
    std::error_code errC;

    SECTION("Generic non-existant negative value") {
        errC = static_cast<foeSplitThreadResult>(INT_MIN);

        CHECK(errC.value() == INT_MIN);
        CHECK(errC.message() == "(unrecognized negative foeSplitThreadResult value)");
    }
    SECTION("Generic non-existant positive value") {
        errC = static_cast<foeSplitThreadResult>(INT_MAX);

        CHECK(errC.value() == INT_MAX);
        CHECK(errC.message() == "(unrecognized positive foeSplitThreadResult value)");
    }

    ERROR_CODE_CATCH_CHECK(FOE_THREAD_POOL_SUCCESS)
    ERROR_CODE_CATCH_CHECK(FOE_THREAD_POOL_ERROR_ZERO_SYNC_THREADS)
    ERROR_CODE_CATCH_CHECK(FOE_THREAD_POOL_ERROR_ZERO_ASYNC_THREADS)
    ERROR_CODE_CATCH_CHECK(FOE_THREAD_POOL_ERROR_ALLOCATION_FAILED)
    ERROR_CODE_CATCH_CHECK(FOE_THREAD_POOL_ERROR_ALREADY_STARTED)
    ERROR_CODE_CATCH_CHECK(FOE_THREAD_POOL_ERROR_NOT_STARTED)
}