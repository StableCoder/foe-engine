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

#define ERROR_CODE_CATCH_CHECK(ERROR_NAME, val)                                                    \
    SECTION(#ERROR_NAME) {                                                                         \
        errC = ERROR_NAME;                                                                         \
                                                                                                   \
        CHECK(errC.value() == ERROR_NAME);                                                         \
        CHECK(errC.message() == #ERROR_NAME);                                                      \
        CHECK(std::string_view{errC.category().name()} == "XrResult");                             \
    }

TEST_CASE("XrResult - Ensure error codes return correct values and strings") {
    std::error_code errC;

    SECTION("Generic non-existant negative value") {
        errC = static_cast<XrResult>(FOE_RESULT_MIN_ENUM);

        CHECK(errC.value() == FOE_RESULT_MIN_ENUM);
        CHECK(errC.message() == "(unrecognized negative XrResult value)");
    }
    SECTION("Generic non-existant positive value") {
        errC = static_cast<XrResult>(FOE_RESULT_MAX_ENUM - 1);

        CHECK(errC.value() == FOE_RESULT_MAX_ENUM - 1);
        CHECK(errC.message() == "(unrecognized positive XrResult value)");
    }
    SECTION("Generic non-existant positive value") {
        errC = static_cast<XrResult>(FOE_RESULT_MAX_ENUM);

        CHECK(errC.value() == FOE_RESULT_MAX_ENUM);
        CHECK(errC.message() == "XR_RESULT_MAX_ENUM");
    }

    XR_LIST_ENUM_XrResult(ERROR_CODE_CATCH_CHECK)
}