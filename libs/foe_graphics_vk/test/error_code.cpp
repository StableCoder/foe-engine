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

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        errC = X;                                                                                  \
                                                                                                   \
        CHECK(errC.value() == X);                                                                  \
        CHECK(errC.message() == #X);                                                               \
        CHECK(std::string_view{errC.category().name()} == "foeGraphicsVkResult");                  \
    }

TEST_CASE("Ensure error codes return correct values and strings") {
    std::error_code errC;

    SECTION("Generic non-existant negative value") {
        errC = static_cast<foeGraphicsVkResult>(FOE_RESULT_MIN_ENUM);

        CHECK(errC.value() == FOE_RESULT_MIN_ENUM);
        CHECK(errC.message() == "FOE_GRAPHICS_VK_UNKNOWN_ERROR_2147483647");
    }
    SECTION("Generic non-existant positive value") {
        errC = static_cast<foeGraphicsVkResult>(FOE_RESULT_MAX_ENUM);

        CHECK(errC.value() == FOE_RESULT_MAX_ENUM);
        CHECK(errC.message() == "FOE_GRAPHICS_VK_UNKNOWN_SUCCESS_2147483647");
    }

    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_SUCCESS)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_INCOMPLETE)
    // Session
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_SESSION_UNKNOWN_FEATURE_STRUCT)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_SESSION_RUNTIME_NOT_SUPPORT_FEATURE_STRUCT)
    // RenderTarget
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_TARGET_NO_COMPATIBLE_RENDER_PASS)
    // RenderGraph - BlitJob
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_SOURCE_NOT_IMAGE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_SOURCE_NO_STATE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_DESTINATION_NOT_IMAGE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_DESTINATION_NOT_MUTABLE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_DESTINATION_NO_STATE)
    // RenderGraph - ResolveJob
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_SOURCE_NOT_IMAGE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_SOURCE_NO_STATE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_DESTINATION_NOT_IMAGE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_DESTINATION_NOT_MUTABLE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_DESTINATION_NO_STATE)
    // RenderGraph - ExportImage
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_EXPORT_IMAGE_RESOURCE_NOT_IMAGE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_EXPORT_IMAGE_RESOURCE_NO_STATE)
    // RenderGraph - PresentSwapchainImage
    ERROR_CODE_CATCH_CHECK(
        FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_PRESENT_SWAPCHAIN_RESOURCE_NOT_SWAPCHAIN)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_PRESENT_SWAPCHAIN_RESOURCE_NO_STATE)
}