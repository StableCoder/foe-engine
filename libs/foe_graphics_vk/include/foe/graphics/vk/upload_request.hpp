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

#ifndef FOE_GRAPHICS_VK_UPLOAD_REQUEST_HPP
#define FOE_GRAPHICS_VK_UPLOAD_REQUEST_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/upload_request.hpp>
#include <vulkan/vulkan.h>

/**
 * @brief Returns the status of an upload request
 * @param uploadRequest Request to get the completion status for
 * @return VK_SUCCESS if the request hasn't been submitted, or if it has and has finished. An
 * appropriate value otherwise.
 */
FOE_GFX_EXPORT VkResult foeGfxGetUploadRequestStatus(foeGfxUploadRequest uploadRequest);

#endif // FOE_GRAPHICS_VK_UPLOAD_REQUEST_HPP