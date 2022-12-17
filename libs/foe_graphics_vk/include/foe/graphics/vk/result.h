// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_RESULT_H
#define FOE_GRAPHICS_VK_RESULT_H

#include <foe/graphics/export.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeGraphicsVkResult {
    FOE_GRAPHICS_VK_SUCCESS = 0,
    FOE_GRAPHICS_VK_INCOMPLETE = 1000010001,
    FOE_GRAPHICS_VK_NO_JOBS_TO_COMPILE = 1000010002,
    FOE_GRAPHICS_VK_NO_JOBS_TO_EXECUTE = 1000010003,
    FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY = -1000010001,
    FOE_GRAPHICS_VK_ERROR_SESSION_UNKNOWN_FEATURE_STRUCT = -1000010002,
    FOE_GRAPHICS_VK_ERROR_SESSION_RUNTIME_NOT_SUPPORT_FEATURE_STRUCT = -1000010003,
    FOE_GRAPHICS_VK_ERROR_RENDER_TARGET_NO_COMPATIBLE_RENDER_PASS = -1000010004,
    FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_SOURCE_NOT_IMAGE = -1000010005,
    FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_SOURCE_NO_STATE = -1000010006,
    FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_DESTINATION_NOT_IMAGE = -1000010007,
    FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_DESTINATION_NOT_MUTABLE = -1000010008,
    FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_BLIT_DESTINATION_NO_STATE = -1000010009,
    FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_SOURCE_NOT_IMAGE = -1000010010,
    FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_SOURCE_NO_STATE = -1000010011,
    FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_DESTINATION_NOT_IMAGE = -1000010012,
    FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_DESTINATION_NOT_MUTABLE = -1000010013,
    FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_RESOLVE_DESTINATION_NO_STATE = -1000010014,
    FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_EXPORT_IMAGE_RESOURCE_NOT_IMAGE = -1000010015,
    FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_EXPORT_IMAGE_RESOURCE_NO_STATE = -1000010016,
    FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_PRESENT_SWAPCHAIN_RESOURCE_NOT_SWAPCHAIN = -1000010017,
    FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_PRESENT_SWAPCHAIN_RESOURCE_NO_STATE = -1000010018,
    FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_NOT_COMPILED = -1000010019,
    FOE_GRAPHICS_VK_ERROR_NO_PROVIDED_SHADER_CODE = -100001020,
    FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_INCOMPATIBLE_STATE = -100001021,
    FOE_GRAPHICS_VK_ERROR_RENDER_GRAPH_IMMUTABLE_RESOURCE = -100001022,
    FOE_GRAPHICS_VK_ERROR_CHAIN_SIZE_LESS_THAN_SUPPORTED = -100001023,
    FOE_GRAPHICS_VK_ERROR_CHAIN_SIZE_MORE_THAN_SUPPORTED = -100001024,
    FOE_GRAPHICS_VK_ERROR_CHAIN_SIZE_DIFFERS = -100001025,
    FOE_GRAPHICS_VK_ERROR_OUT_OF_POOL_MEMORY = -100001026,
    FOE_GRAPHICS_VK_ERROR_DATA_LARGER_THAN_BUFFER = -100001027,
} foeGraphicsVkResult;

FOE_GFX_EXPORT void foeGraphicsVkResultToString(foeGraphicsVkResult value,
                                                char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_VK_RESULT_H