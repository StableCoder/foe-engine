// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>

#include <foe/imgui/vk/result.h>

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