/*
    Copyright (C) 2020 George Cave.

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

#ifndef FOE_GRAPHICS_ERROR_COLOUR_IMAGE_HPP
#define FOE_GRAPHICS_ERROR_COLOUR_IMAGE_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/upload_context.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

FOE_GFX_EXPORT VkResult foeCreateErrorColourImage(foeGfxUploadContext uploadContext,
                                                  VkFormat format,
                                                  uint32_t numMipLevels,
                                                  uint32_t numCheckSquares,
                                                  VmaAllocation *pAlloc,
                                                  VkImage *pImage,
                                                  VkImageView *pImageView,
                                                  VkSampler *pSampler);

FOE_GFX_EXPORT VkResult foeCreateErrorDepthStencilImage(foeGfxUploadContext uploadContext,
                                                        uint32_t numMipLevels,
                                                        uint32_t numCheckSquares,
                                                        VmaAllocation *pAlloc,
                                                        VkImage *pImage,
                                                        VkImageView *pImageDepthView,
                                                        VkImageView *pImageStencilView,
                                                        VkSampler *pSampler);

#endif // FOE_GRAPHICS_ERROR_COLOUR_IMAGE_HPP