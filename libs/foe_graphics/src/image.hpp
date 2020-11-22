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

#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <vulkan/vulkan.h>

#include <cmath>
#include <vector>

struct foeResourceUploader;
struct UploadData;

/** @brief Returns the extent for a given mip level.
 * @param extent The original extent for mip level 0.
 * @param level The mip level to retrieve. Must be less than 32.
 * @return The extent for the given mip level.
 * @warning There is no safety for a mipmap level beyond the max, which would also return (1, 1).
 */
VkExtent3D mipmapExtent(VkExtent3D extent, uint32_t mipLevel) noexcept;

/** Returns the pixel count for given extent, over the number of given mipmap levels.
 * @param extent The extent of the base image.
 * @param levels The number of levels down to count the number of pixels for.
 * @return The overall pixel count.
 */
VkDeviceSize pixelCount(VkExtent3D extent, uint32_t mipLevels) noexcept;

VkResult recordImageUploadCommands(foeResourceUploader *pResourceUploader,
                                   VkImageSubresourceRange const *pSubresourceRange,
                                   std::vector<VkBufferImageCopy> const *pCopyRegions,
                                   VkBuffer srcBuffer,
                                   VkImage dstImage,
                                   VkAccessFlags dstAccessFlags,
                                   VkImageLayout dstImageLayout,
                                   UploadData *pUploadData);

#endif // IMAGE_HPP