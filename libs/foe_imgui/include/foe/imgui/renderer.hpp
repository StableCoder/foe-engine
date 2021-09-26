/*
    Copyright (C) 2020-2021 George Cave.

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

#ifndef FOE_IMGUI_RENDERER_HPP
#define FOE_IMGUI_RENDERER_HPP

#include <foe/graphics/session.hpp>
#include <foe/imgui/export.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <system_error>
#include <vector>

struct ImGuiContext;
struct foeWsiKeyboard;
struct foeWsiMouse;

class foeImGuiRenderer {
  public:
    FOE_IMGUI_EXPORT void setImGuiContext(ImGuiContext *pContext);

    FOE_IMGUI_EXPORT auto initialize(foeGfxSession session,
                                     VkSampleCountFlags rasterSampleFlags,
                                     VkRenderPass renderPass,
                                     uint32_t subpass) -> std::error_code;
    FOE_IMGUI_EXPORT void deinitialize(foeGfxSession session);
    FOE_IMGUI_EXPORT bool initialized() const noexcept;

    FOE_IMGUI_EXPORT void newFrame();
    FOE_IMGUI_EXPORT void endFrame();

    FOE_IMGUI_EXPORT auto update(uint32_t bufferedFrame) -> std::error_code;
    FOE_IMGUI_EXPORT void draw(VkCommandBuffer commandBuffer, uint32_t frameIndex);

    FOE_IMGUI_EXPORT void resize(float width, float height);
    FOE_IMGUI_EXPORT void rescale(float xScale, float yScale);

    FOE_IMGUI_EXPORT bool wantCaptureMouse() const noexcept;
    FOE_IMGUI_EXPORT void mouseInput(foeWsiMouse const *pMouse) noexcept;

    FOE_IMGUI_EXPORT bool wantCaptureKeyboard() const noexcept;
    FOE_IMGUI_EXPORT void keyboardInput(foeWsiKeyboard const *pKeyboard) noexcept;

  private:
    VkResult initializeDescriptorPool(VkDevice device);
    VkResult initializeDescriptorSetLayout(VkDevice device);
    VkResult initializeDescriptorSet(VkDevice device);
    VkResult initializePipelineLayout(VkDevice device);
    VkResult initializePipeline(VkDevice device,
                                VkSampleCountFlags rasterSampleFlags,
                                VkRenderPass renderPass,
                                uint32_t subpass);

    struct BufferSet {
        VkBuffer buffer{VK_NULL_HANDLE};
        VmaAllocation alloc{VK_NULL_HANDLE};
        uint32_t count{0};
    };

    struct DrawBuffers {
        BufferSet vertices;
        BufferSet indices;
    };

    foeGfxSession mGfxSession{FOE_NULL_HANDLE};

    VkImageView mFontView{VK_NULL_HANDLE};
    VkSampler mFontSampler{VK_NULL_HANDLE};
    VkImage mFontImage{VK_NULL_HANDLE};
    VmaAllocation mFontAlloc;

    VkDescriptorSetLayout mDescriptorSetLayout{VK_NULL_HANDLE};
    VkDescriptorPool mDescriptorPool{VK_NULL_HANDLE};
    VkDescriptorSet mDescriptorSet;

    VkPipelineLayout mPipelineLayout{VK_NULL_HANDLE};
    VkPipeline mPipeline{VK_NULL_HANDLE};

    std::vector<DrawBuffers> mDrawBuffers;
};

#endif // FOE_IMGUI_RENDERER_HPP