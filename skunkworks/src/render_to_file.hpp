#ifndef FOE_RENDER_TO_FILE_HPP
#define FOE_RENDER_TO_FILE_HPP

#include <foe/graphics/session.h>
#include <foe/graphics/vk/render_graph.hpp>
#include <foe/graphics/vk/session.h>
#include <foe/split_thread_pool.h>

#include <string>

struct CpuImageData {
    foeGfxSession gfxSession;

    VmaAllocation alloc;
    VkImage image;
    VkFormat format;

    VkExtent3D extent;
    VkFence fence;

    std::string filename;
};

#ifdef __cplusplus
extern "C" {
#endif

CpuImageData *addCpuImageExport(foeGfxSession gfxSession,
                                foeGfxVkRenderGraph gfxVkRenderGraph,
                                VkExtent2D extent,
                                char const *pFilename,
                                foeGfxVkRenderGraphResource imageToExport,
                                VkSampleCountFlags incomingImageSampleCount,
                                foeGfxVkRenderGraphJob lastImageJob);

void renderedImageToFile(void *pData);

#ifdef __cplusplus
}
#endif

#endif // FOE_RENDER_TO_FILE_HPP