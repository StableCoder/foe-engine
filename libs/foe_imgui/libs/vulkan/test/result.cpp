// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/imgui/vk/result.h>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        foeImGuiVkResultToString(X, resultString);                                                 \
        CHECK(std::string{resultString} == #X);                                                    \
    }

TEST_CASE("foeImGuiVkResult - Ensure error codes return correct values and strings") {
    char resultString[FOE_MAX_RESULT_STRING_SIZE];

    SECTION("Generic non-existant negative value") {
        foeImGuiVkResultToString((foeImGuiVkResult)FOE_RESULT_MIN_ENUM, resultString);
        CHECK(std::string{resultString} == "FOE_IM_GUI_VK_UNKNOWN_ERROR_2147483647");
    }
    SECTION("Generic non-existant positive value") {
        foeImGuiVkResultToString((foeImGuiVkResult)FOE_RESULT_MAX_ENUM, resultString);
        CHECK(std::string{resultString} == "FOE_IM_GUI_VK_UNKNOWN_SUCCESS_2147483647");
    }

    ERROR_CODE_CATCH_CHECK(FOE_IMGUI_VK_SUCCESS)
    ERROR_CODE_CATCH_CHECK(FOE_IMGUI_VK_ERROR_OUT_OF_MEMORY)
    ERROR_CODE_CATCH_CHECK(FOE_IMGUI_VK_ERROR_GRAPH_UI_COLOUR_TARGET_NOT_IMAGE)
    ERROR_CODE_CATCH_CHECK(FOE_IMGUI_VK_ERROR_GRAPH_UI_COLOUR_TARGET_NOT_MUTABLE)
    ERROR_CODE_CATCH_CHECK(FOE_IMGUI_VK_ERROR_GRAPH_UI_COLOUR_TARGET_MISSING_STATE)
}
