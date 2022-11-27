// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/graphics/vk/result.h>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        foeGraphicsVkResultToString(X, resultString);                                              \
        CHECK(std::string_view{resultString} == #X);                                               \
    }

TEST_CASE("foeGraphicsVkResult - Ensure error codes return correct values and strings") {
    char resultString[FOE_MAX_RESULT_STRING_SIZE];

    SECTION("Generic non-existant negative value") {
        foeGraphicsVkResultToString((foeGraphicsVkResult)FOE_RESULT_MIN_ENUM, resultString);
        CHECK(std::string_view{resultString} == "FOE_GRAPHICS_VK_UNKNOWN_ERROR_2147483647");
    }
    SECTION("Generic non-existant positive value") {
        foeGraphicsVkResultToString((foeGraphicsVkResult)FOE_RESULT_MAX_ENUM, resultString);
        CHECK(std::string_view{resultString} == "FOE_GRAPHICS_VK_UNKNOWN_SUCCESS_2147483647");
    }

    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_SUCCESS)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_INCOMPLETE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_NO_JOBS_TO_COMPILE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_NO_JOBS_TO_EXECUTE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_SESSION_UNKNOWN_FEATURE_STRUCT)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_SESSION_RUNTIME_NOT_SUPPORT_FEATURE_STRUCT)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_TARGET_NO_COMPATIBLE_RENDER_PASS)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_SOURCE_NOT_IMAGE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_SOURCE_NO_STATE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_DESTINATION_NOT_IMAGE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_DESTINATION_NOT_MUTABLE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_DESTINATION_NO_STATE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_SOURCE_NOT_IMAGE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_SOURCE_NO_STATE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_DESTINATION_NOT_IMAGE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_DESTINATION_NOT_MUTABLE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_DESTINATION_NO_STATE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_EXPORT_IMAGE_RESOURCE_NOT_IMAGE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_EXPORT_IMAGE_RESOURCE_NO_STATE)
    ERROR_CODE_CATCH_CHECK(
        FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_PRESENT_SWAPCHAIN_RESOURCE_NOT_SWAPCHAIN)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_PRESENT_SWAPCHAIN_RESOURCE_NO_STATE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_NOT_COMPILED)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_NO_PROVIDED_SHADER_CODE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_IMMUTABLE_RESOURCE)
}
