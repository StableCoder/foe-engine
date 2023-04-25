// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_RENDER_PASS_H
#define FOE_GRAPHICS_VK_RENDER_PASS_H

#include <foe/graphics/export.h>
#include <foe/graphics/session.h>
#include <foe/handle.h>
#include <foe/result.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_DEFINE_HANDLE(foeGfxVkRenderPassPool)

/**
 * @brief Returns a specific render pass for the given attachment
 * @param attachments Describes the attachment set the render pass is defined by
 * @return A valid render pass handle, VK_NULL_HANDLE otherwise
 *
 * If the render pass described already exists, this will return the handle to that. If it
 * doesn't it will be generated.
 */
FOE_GFX_EXPORT
VkRenderPass foeGfxVkGetRenderPass(foeGfxVkRenderPassPool renderPassPool,
                                   uint32_t attachmentCount,
                                   VkAttachmentDescription const *pAttachments);

/**
 * @brief Returns a compatible render pass for the given formats/samples
 * @param formats Image formats corresponding to the attached images for the render pass
 * @param samples Sample counts corresponding to the attached images for the render pass
 * @return First compatible render pass handle, VK_NULL_HANDLE otherwise
 * @note This is intended for finding render passes quickly for use in geneating framebuffers
 * rather than rendering.
 *
 * Thanks to the compatibility of render passes based on a minimal set of the formnats and
 * sample count, this function will attempt to find *any* compatible render passes and return
 * the first found.
 *
 * If no render pass is found, it will generate a basic one instead.
 */
FOE_GFX_EXPORT
VkRenderPass foeGfxVkGetCompatibleRenderPass(foeGfxVkRenderPassPool renderPassPool,
                                             uint32_t attachmentCount,
                                             VkFormat const *pFormats,
                                             VkSampleCountFlags const *pSampleFlags);

#ifdef __cplusplus
}
#endif

#endif // FOE_GRAPHICS_VK_RENDER_PASS_H