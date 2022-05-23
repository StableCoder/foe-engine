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

#include <foe/imex/yaml/error_code.h>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        foeImexYamlResultToString(X, resultString);                                                \
        CHECK(std::string_view{resultString} == #X);                                               \
    }

TEST_CASE("foeImexYamlResult - Ensure error codes return correct values and strings") {
    char resultString[FOE_MAX_RESULT_STRING_SIZE];

    SECTION("Generic non-existant negative value") {
        foeImexYamlResultToString((foeImexYamlResult)FOE_RESULT_MIN_ENUM, resultString);
        CHECK(std::string_view{resultString} == "FOE_IMEX_YAML_UNKNOWN_ERROR_2147483647");
    }
    SECTION("Generic non-existant positive value") {
        foeImexYamlResultToString((foeImexYamlResult)FOE_RESULT_MAX_ENUM, resultString);
        CHECK(std::string_view{resultString} == "FOE_IMEX_YAML_UNKNOWN_SUCCESS_2147483647");
    }

    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_SUCCESS)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_INCOMPLETE)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_FUNCTIONALITY_ALREADY_REGISTERED)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_FUNCTIONALITY_NOT_REGISTERED)
    // Importer
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_PATH_NOT_DIRECTORY)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_DEPENDENCIES_FILE_NOT_EXIST)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_DEPENDENCIES_FILE_NOT_REGULAR_FILE)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_FAILED_TO_READ_DEPENDENCIES)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_RESOURCE_INDEX_FILE_NOT_EXIST)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_RESOURCE_INDEX_FILE_NOT_REGULAR_FILE)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_ENTITY_INDEX_FILE_NOT_EXIST)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_ENTITY_INDEX_FILE_NOT_REGULAR_FILE)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_RESOURCE_DIRECTORY_NOT_DIRECTORY)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_ENTITY_DIRECTORY_NOT_DIRECTORY)
    ERROR_CODE_CATCH_CHECK(FOE_IMEX_YAML_ERROR_EXTERNAL_DIRECTORY_NOT_DIRECTORY)
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