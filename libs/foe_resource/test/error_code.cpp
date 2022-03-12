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

#include <catch.hpp>

#include "../src/error_code.hpp"

#include <climits>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        errC = X;                                                                                  \
                                                                                                   \
        CHECK(errC.value() == X);                                                                  \
        CHECK(errC.message() == #X);                                                               \
    }

TEST_CASE("foeResourceResult - Ensure error codes return correct values and strings") {
    std::error_code errC;

    SECTION("Generic non-existant negative value") {
        errC = static_cast<foeResourceResult>(INT_MIN);

        CHECK(errC.value() == INT_MIN);
        CHECK(errC.message() == "(unrecognized negative foeResourceResult value)");
    }
    SECTION("Generic non-existant positive value") {
        errC = static_cast<foeResourceResult>(INT_MAX);

        CHECK(errC.value() == INT_MAX);
        CHECK(errC.message() == "(unrecognized positive foeResourceResult value)");
    }

    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_SUCCESS)
    // General
    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_ERROR_OUT_OF_HOST_MEMORY)
    // Resource specific
    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_ERROR_RESOURCE_FUNCTIONS_NOT_PROVIDED)
}