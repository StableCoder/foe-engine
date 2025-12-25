#include "render_to_file.hpp"

#include <MagickCore/MagickCore.h>
#include <foe/graphics/vk/render_graph.hpp>
#include <foe/graphics/vk/render_graph/job/blit_image.hpp>
#include <foe/graphics/vk/render_graph/job/export_image.hpp>
#include <foe/graphics/vk/render_graph/job/import_image.hpp>
#include <foe/graphics/vk/render_graph/job/resolve_image.hpp>

#include "log.hpp"

#include <cstring>
#include <fstream>

extern "C" CpuImageData *addCpuImageExport(foeGfxSession gfxSession,
                                           foeGfxVkRenderGraph gfxVkRenderGraph,
                                           VkExtent2D extent,
                                           char const *pFilename,
                                           foeGfxVkRenderGraphResource imageToExport,
                                           VkSampleCountFlags incomingImageSampleCount,
                                           foeGfxVkRenderGraphJob lastImageJob) {
    CpuImageData *pCpuImageData = new CpuImageData{
        .gfxSession = gfxSession,
        .filename = pFilename,
    };

    // Create Fence
    VkFenceCreateInfo fenceCI{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };

    VkResult vkRes =
        vkCreateFence(foeGfxVkGetDevice(gfxSession), &fenceCI, nullptr, &pCpuImageData->fence);
    if (vkRes != VK_SUCCESS) {
        std::abort();
    }

    // Create Image
    pCpuImageData->format = VK_FORMAT_B8G8R8A8_UNORM;
    pCpuImageData->extent = VkExtent3D{
        .width = extent.width,
        .height = extent.height,
        .depth = 1U,
    };

    VkImageCreateInfo imageCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = pCpuImageData->format,
        .extent = pCpuImageData->extent,
        .mipLevels = 1U,
        .arrayLayers = 1U,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_LINEAR,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
    };

    VmaAllocationCreateInfo allocCI{
        .usage = VMA_MEMORY_USAGE_CPU_ONLY,
    };

    vkRes = vmaCreateImage(foeGfxVkGetAllocator(gfxSession), &imageCI, &allocCI,
                           &pCpuImageData->image, &pCpuImageData->alloc, nullptr);
    if (vkRes != VK_SUCCESS) {
        std::abort();
    }

    foeGfxVkRenderGraphResource cpuCopiedImage;
    foeGfxVkRenderGraphJob cpuImageImportJob;
    foeResultSet result = foeGfxVkImportImageRenderJob(
        gfxVkRenderGraph, "importCpuImage", VK_NULL_HANDLE, "cpuImage", pCpuImageData->image,
        VK_NULL_HANDLE, pCpuImageData->format, extent, VK_IMAGE_LAYOUT_UNDEFINED, true, {},
        &cpuCopiedImage, &cpuImageImportJob);
    if (result.value != FOE_SUCCESS) {
        std::abort();
    }

    foeGfxVkRenderGraphJob cpuResolveOrCopyJob;
    if (incomingImageSampleCount != VK_SAMPLE_COUNT_1_BIT) {
        // Resolve
        result = foeGfxVkResolveImageRenderJob(
            gfxVkRenderGraph, "resolveRenderedImageToCpuImage", VK_NULL_HANDLE, imageToExport, 1,
            &lastImageJob, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, cpuCopiedImage, 1,
            &cpuImageImportJob, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &cpuResolveOrCopyJob);
        if (result.value != FOE_SUCCESS) {
            std::abort();
        }
    } else {
        // Copy
        result = foeGfxVkBlitImageRenderJob(
            gfxVkRenderGraph, "blitRenderedImageToCpuImage", VK_NULL_HANDLE, imageToExport, 1,
            &lastImageJob, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, cpuCopiedImage, 1,
            &cpuImageImportJob, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_FILTER_NEAREST,
            &cpuResolveOrCopyJob);
        if (result.value != FOE_SUCCESS) {
            std::abort();
        }
    }

    foeGfxVkExportImageRenderJob(gfxVkRenderGraph, "exportCpuImage", pCpuImageData->fence,
                                 cpuCopiedImage, 1, &cpuResolveOrCopyJob,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, {});

    return pCpuImageData;
}

extern "C" void renderedImageToFile(void *pContext) {
    CpuImageData *pCpuImageData = (CpuImageData *)pContext;

    ExceptionInfo *exceptionInfo;
    exceptionInfo = AcquireExceptionInfo();

    void *pData = nullptr;
    VkResult vkRes =
        vmaMapMemory(foeGfxVkGetAllocator(pCpuImageData->gfxSession), pCpuImageData->alloc, &pData);
    if (vkRes != VK_SUCCESS) {
        std::abort();
    }

    Image *image = ConstituteImage(pCpuImageData->extent.width, pCpuImageData->extent.height,
                                   "BGRA", CharPixel, pData, exceptionInfo);

    vmaUnmapMemory(foeGfxVkGetAllocator(pCpuImageData->gfxSession), pCpuImageData->alloc);

    vkDestroyFence(foeGfxVkGetDevice(pCpuImageData->gfxSession), pCpuImageData->fence, nullptr);

    vmaDestroyImage(foeGfxVkGetAllocator(pCpuImageData->gfxSession), pCpuImageData->image,
                    pCpuImageData->alloc);

    ImageInfo *imageInfo = AcquireImageInfo();
    strcpy(imageInfo->filename, pCpuImageData->filename.c_str());

    size_t blobSize = 0;
    void *blob = ImageToBlob(imageInfo, image, &blobSize, exceptionInfo);

    std::ofstream outFile(pCpuImageData->filename, std::ofstream::out | std::ofstream::binary);
    outFile.write((char const *)blob, blobSize);

    image = DestroyImage(image);
    imageInfo = DestroyImageInfo(imageInfo);

    exceptionInfo = DestroyExceptionInfo(exceptionInfo);

    std::string logMsg = "SAVED IMAGE: " + pCpuImageData->filename;
    FOE_LOG(foeBringup, FOE_LOG_LEVEL_INFO, logMsg.c_str());

    delete pCpuImageData;
    pCpuImageData = nullptr;
}