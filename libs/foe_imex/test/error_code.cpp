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

#include "../src/error_code.hpp"

#include <climits>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        errC = X;                                                                                  \
                                                                                                   \
        CHECK(errC.value() == X);                                                                  \
        CHECK(errC.message() == #X);                                                               \
    }

TEST_CASE("foeImexResult - Ensure error codes return correct values and strings") {
    std::error_code errC;

    SECTION("Generic non-existant negative value") {
        errC = static_cast<foeImexResult>(INT_MIN);

        CHECK(errC.value() == INT_MIN);
        CHECK(errC.message() == "(unrecognized negative foeImexResult value)");
    }
    SECTION("Generic non-existant positive value") {
        errC = static_cast<foeImexResult>(INT_MAX);

        CHECK(errC.value() == INT_MAX);
        CHECK(errC.message() == "(unrecognized positive foeImexResult value)");
    }

    ERROR_CODE_CATCH_CHECK(FOE_IMEX_SUCCESS)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_ERROR_FUNCTIONALITY_ALREADY_REGISTERED)
    // Exporter
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_ERROR_EXPORTER_ALREADY_REGISTERED)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_ERROR_EXPORTER_NOT_REGISTERED)
    // Importer
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_ERROR_IMPORTER_ALREADY_REGISTERED)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_ERROR_IMPORTER_NOT_REGISTERED)
}