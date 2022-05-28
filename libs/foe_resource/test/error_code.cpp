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
#include <foe/resource/error_code.h>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        foeResourceResultToString(X, resultString);                                                \
        CHECK(std::string_view{resultString} == #X);                                               \
    }

TEST_CASE("foeResourceResult - Ensure error codes return correct values and strings") {
    char resultString[FOE_MAX_RESULT_STRING_SIZE];

    SECTION("Generic non-existant negative value") {
        foeResourceResultToString((foeResourceResult)FOE_RESULT_MIN_ENUM, resultString);
        CHECK(std::string_view{resultString} == "FOE_RESOURCE_UNKNOWN_ERROR_2147483647");
    }
    SECTION("Generic non-existant positive value") {
        foeResourceResultToString((foeResourceResult)FOE_RESULT_MAX_ENUM, resultString);
        CHECK(std::string_view{resultString} == "FOE_RESOURCE_UNKNOWN_SUCCESS_2147483647");
    }

    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_SUCCESS)
    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_ERROR_NOT_FOUND)
    // General
    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_ERROR_OUT_OF_HOST_MEMORY)
    // Records
    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_CANNOT_UNDO)
    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_CANNOT_REDO)
    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_NO_MODIFIED_RECORD)
    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_ERROR_EXISTING_RECORD)
    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_ERROR_NON_EXISTING_RECORD)
    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_ERROR_CANNOT_REMOVE_NON_PERSISTENT_RECORDS)
    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_ERROR_NO_RECORDS)
    // Resource Specific
    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_ERROR_RESOURCE_FUNCTIONS_NOT_PROVIDED)
    // CreateInfo Specifc
    ERROR_CODE_CATCH_CHECK(FOE_RESOURCE_ERROR_DATA_FUNCTION_NOT_PROVIDED)
}