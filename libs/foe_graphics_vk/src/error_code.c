// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/error_code.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foeGraphicsVkResultToString(foeGraphicsVkResult value,
                                 char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
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
        if (value > 0) {
            sprintf(buffer, "FOE_GRAPHICS_VK_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "FOE_GRAPHICS_VK_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}