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
        CHECK(std::string_view{errC.category().name()} == "foeImexYamlResult");                    \
    }

TEST_CASE("foeImexYamlResult - Ensure error codes return correct values and strings") {
    std::error_code errC;

    SECTION("Generic non-existant negative value") {
        errC = static_cast<foeImexYamlResult>(INT_MIN);

        CHECK(errC.value() == INT_MIN);
        CHECK(errC.message() == "(unrecognized negative foeImexYamlResult value)");
    }
    SECTION("Generic non-existant positive value") {
        errC = static_cast<foeImexYamlResult>(INT_MAX);

        CHECK(errC.value() == INT_MAX);
        CHECK(errC.message() == "(unrecognized positive foeImexYamlResult value)");
    }

    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_SUCCESS)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_FUNCTIONALITY_ALREADY_REGISTERED)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_FUNCTIONALITY_NOT_REGISTERED)
    // Exporter
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_EXPORTER_ALREADY_REGISTERED)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_EXPORTER_NOT_REGISTERED)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_DESTINATION_NOT_DIRECTORY)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_FAILED_TO_PERFORM_FILESYSTEM_OPERATION)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_DEPENDENCIES)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_RESOURCE_INDEX_DATA)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_COMPONENT_INDEX_DATA)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_RESOURCE_DATA)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_COMPONENT_DATA)
}