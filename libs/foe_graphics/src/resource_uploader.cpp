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

#include <foe/graphics/resource_uploader.hpp>

VkResult foeGfxCreateResourceUploader(foeGfxEnvironment *pGfxEnvironment,
                                      foeResourceUploader *pResourceUploader) {
    VkResult res;
    foeResourceUploader resUploader{};

    resUploader.device = pGfxEnvironment->device;
    resUploader.allocator = pGfxEnvironment->allocator;

    auto transferQueue = foeGfxGetBestQueue(pGfxEnvironment, VK_QUEUE_TRANSFER_BIT);
    auto graphicsQueue = foeGfxGetBestQueue(pGfxEnvironment, VK_QUEUE_GRAPHICS_BIT);

    if (transferQueue == graphicsQueue) {
        resUploader.dstQueueFamily = &pGfxEnvironment->pQueueFamilies[graphicsQueue];
    } else {
        resUploader.srcQueueFamily = &pGfxEnvironment->pQueueFamilies[transferQueue];
        resUploader.dstQueueFamily = &pGfxEnvironment->pQueueFamilies[graphicsQueue];
    }

    VkCommandPoolCreateInfo cmdPoolCI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
    };

    if (resUploader.srcQueueFamily != nullptr) {
        cmdPoolCI.queueFamilyIndex = transferQueue;

        res = vkCreateCommandPool(resUploader.device, &cmdPoolCI, nullptr,
                                  &resUploader.srcCommandPool);
        if (res != VK_SUCCESS)
            goto CREATE_FAILED;
    }

    if (resUploader.dstQueueFamily != nullptr) {
        cmdPoolCI.queueFamilyIndex = graphicsQueue;

        res = vkCreateCommandPool(resUploader.device, &cmdPoolCI, nullptr,
                                  &resUploader.dstCommandPool);
        if (res != VK_SUCCESS)
            goto CREATE_FAILED;
    }

    *pResourceUploader = resUploader;

CREATE_FAILED:
    if (res != VK_SUCCESS) {
        foeGfxDestroyResourceUploader(&resUploader);
    }

    return res;
}

void foeGfxDestroyResourceUploader(foeResourceUploader *pResourceUploader) {
    if (pResourceUploader->dstCommandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(pResourceUploader->device, pResourceUploader->dstCommandPool, nullptr);
    }

    if (pResourceUploader->srcCommandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(pResourceUploader->device, pResourceUploader->srcCommandPool, nullptr);
    }
}