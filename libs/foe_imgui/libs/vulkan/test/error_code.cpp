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

#include <foe/imgui/vk/error_code.h>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        foeImGuiVkResultToString(X, resultString);                                                 \
        CHECK(std::string_view{resultString} == #X);                                               \
    }

TEST_CASE("Ensure error codes return correct values and strings") {
    char resultString[FOE_MAX_RESULT_STRING_SIZE];

    SECTION("Generic non-existant negative value") {
        foeImGuiVkResultToString((foeImGuiVkResult)FOE_RESULT_MIN_ENUM, resultString);
        CHECK(std::string_view{resultString} == "FOE_IMGUI_VK_UNKNOWN_ERROR_2147483647");
    }
    SECTION("Generic non-existant positive value") {
        foeImGuiVkResultToString((foeImGuiVkResult)FOE_RESULT_MAX_ENUM, resultString);
        CHECK(std::string_view{resultString} == "FOE_IMGUI_VK_UNKNOWN_SUCCESS_2147483647");
    }

    ERROR_CODE_CATCH_CHECK(FOE_IMGUI_VK_SUCCESS)
    // RenderGraph - UI Job
    ERROR_CODE_CATCH_CHECK(FOE_IMGUI_VK_GRAPH_UI_COLOUR_TARGET_NOT_IMAGE)
    ERROR_CODE_CATCH_CHECK(FOE_IMGUI_VK_GRAPH_UI_COLOUR_TARGET_NOT_MUTABLE)
    ERROR_CODE_CATCH_CHECK(FOE_IMGUI_VK_GRAPH_UI_COLOUR_TARGET_MISSING_STATE)
}