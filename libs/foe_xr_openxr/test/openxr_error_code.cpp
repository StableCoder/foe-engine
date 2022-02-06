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
#include <foe/xr/error_code.hpp>

#include <climits>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        errC = X;                                                                                  \
                                                                                                   \
        CHECK(errC.value() == X);                                                                  \
        CHECK(errC.message() == #X);                                                               \
    }

TEST_CASE("XrResult - Ensure error codes return correct values and strings") {
    std::error_code errC;

    SECTION("Generic non-existant negative value") {
        errC = static_cast<XrResult>(INT_MIN);

        CHECK(errC.value() == INT_MIN);
        CHECK(errC.message() == "(unrecognized negative XrResult value)");
    }
    SECTION("Generic non-existant positive value") {
        errC = static_cast<XrResult>(INT_MAX - 1);

        CHECK(errC.value() == INT_MAX - 1);
        CHECK(errC.message() == "(unrecognized positive XrResult value)");
    }
    SECTION("Generic non-existant positive value") {
        errC = static_cast<XrResult>(INT_MAX);

        CHECK(errC.value() == INT_MAX);
        CHECK(errC.message() == "XR_RESULT_MAX_ENUM");
    }

    ERROR_CODE_CATCH_CHECK(XR_SUCCESS)
    ERROR_CODE_CATCH_CHECK(XR_TIMEOUT_EXPIRED)
    ERROR_CODE_CATCH_CHECK(XR_ERROR_RUNTIME_FAILURE)
}