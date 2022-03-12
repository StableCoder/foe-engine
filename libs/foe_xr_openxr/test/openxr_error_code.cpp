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
#include <foe/xr/openxr/error_code.hpp>
#include <openxr/openxr_reflection.h>

#include <climits>

#define ERROR_CODE_CATCH_CHECK(name, val)                                                          \
    SECTION(#name) {                                                                               \
        errC = name;                                                                               \
                                                                                                   \
        CHECK(errC.value() == name);                                                               \
        CHECK(errC.message() == #name);                                                            \
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

    XR_LIST_ENUM_XrResult(ERROR_CODE_CATCH_CHECK)
}