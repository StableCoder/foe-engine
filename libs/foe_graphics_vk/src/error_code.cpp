/*
    Copyright (C) 2021 George Cave.

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

#include "error_code.hpp"

namespace {

struct foeGraphicsVkErrCategory : std::error_category {
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *foeGraphicsVkErrCategory::name() const noexcept { return "foeGraphicsVkResult"; }

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        return #X;

std::string foeGraphicsVkErrCategory::message(int ev) const {
    switch (static_cast<foeGraphicsVkResult>(ev)) {
        RESULT_CASE(FOE_GRAPHICS_VK_SUCCESS)
        RESULT_CASE(FOE_GRAPHICS_VK_INCOMPLETE)
        // Session
        RESULT_CASE(FOE_GRAPHICS_VK_ERROR_SESSION_UNKNOWN_FEATURE_STRUCT)
        RESULT_CASE(FOE_GRAPHICS_VK_ERROR_SESSION_RUNTIME_NOT_SUPPORT_FEATURE_STRUCT)
        // RenderTarget
        RESULT_CASE(FOE_GRAPHICS_VK_ERROR_RENDER_TARGET_NO_COMPATIBLE_RENDER_PASS)
        // RenderGraph - BlitJob
        RESULT_CASE(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_SOURCE_NOT_IMAGE)
        RESULT_CASE(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_SOURCE_NO_STATE)
        RESULT_CASE(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_DESTINATION_NOT_IMAGE)
        RESULT_CASE(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_DESTINATION_NOT_MUTABLE)
        RESULT_CASE(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_DESTINATION_NO_STATE)
        // RenderGraph - ResolveJob
        RESULT_CASE(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_SOURCE_NOT_IMAGE)
        RESULT_CASE(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_SOURCE_NO_STATE)
        RESULT_CASE(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_DESTINATION_NOT_IMAGE)
        RESULT_CASE(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_DESTINATION_NOT_MUTABLE)
        RESULT_CASE(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_DESTINATION_NO_STATE)
        // RenderGraph - ExportImage
        RESULT_CASE(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_EXPORT_IMAGE_RESOURCE_NOT_IMAGE)
        RESULT_CASE(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_EXPORT_IMAGE_RESOURCE_NO_STATE)
        // RenderGraph - PresentSwapchainImage
        RESULT_CASE(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_PRESENT_SWAPCHAIN_RESOURCE_NOT_SWAPCHAIN)
        RESULT_CASE(FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_PRESENT_SWAPCHAIN_RESOURCE_NO_STATE)

    default:
        if (ev > 0)
            return "(unrecognized positive foeGraphicsVkResult value)";
        else
            return "(unrecognized negative foeGraphicsVkResult value)";
    }
}

const foeGraphicsVkErrCategory graphicsVkErrCategory{};

} // namespace

std::error_code make_error_code(foeGraphicsVkResult e) {
    return {static_cast<int>(e), graphicsVkErrCategory};
}