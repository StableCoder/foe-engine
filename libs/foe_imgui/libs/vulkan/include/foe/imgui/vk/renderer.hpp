// Copyright (C) 2020-2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_IMGUI_VK_RENDERER_HPP
#define FOE_IMGUI_VK_RENDERER_HPP

#include <foe/external/imgui.h>
#include <foe/external/vk_mem_alloc.h>
#include <foe/graphics/session.h>
#include <foe/imgui/vk/export.h>
#include <foe/result.h>
#include <vulkan/vulkan.h>

#include <vector>

typedef ImGuiKey (*PFN_ImplKeyToImGuiKey)(int keycode, int scancode);

struct ImGuiContext;

class foeImGuiRenderer {
  public:
    FOE_IMGUI_VK_EXPORT
    void setImGuiContext(ImGuiContext *pContext);

    FOE_IMGUI_VK_EXPORT
    foeResultSet initialize(foeGfxSession session,
                            VkSampleCountFlags rasterSampleFlags,
                            VkRenderPass renderPass,
                            uint32_t subpass);
    FOE_IMGUI_VK_EXPORT
    void deinitialize(foeGfxSession session);
    FOE_IMGUI_VK_EXPORT
    bool initialized() const noexcept;

    FOE_IMGUI_VK_EXPORT
    void newFrame();
    FOE_IMGUI_VK_EXPORT
    void endFrame();

    FOE_IMGUI_VK_EXPORT
    foeResultSet update(uint32_t bufferedFrame);
    FOE_IMGUI_VK_EXPORT
    void draw(VkCommandBuffer commandBuffer, uint32_t frameIndex);

    FOE_IMGUI_VK_EXPORT
    void resize(float width, float height);
    FOE_IMGUI_VK_EXPORT
    void rescale(float xScale, float yScale);

    FOE_IMGUI_VK_EXPORT
    bool wantCaptureMouse() const noexcept;
    FOE_IMGUI_VK_EXPORT
    void mouseInput(float posX,
                    float posY,
                    float scrollX,
                    float scrollY,
                    uint32_t const *pPressedButtons,
                    uint32_t pressedButtonCount,
                    uint32_t const *pReleasedButtons,
                    uint32_t releasedButtonCount) noexcept;

    FOE_IMGUI_VK_EXPORT
    bool wantCaptureKeyboard() const noexcept;
    FOE_IMGUI_VK_EXPORT
    void keyboardInput(uint32_t unicodeChar,
                       PFN_ImplKeyToImGuiKey implKeyToImGuiKey,
                       uint32_t const *pPressedKeycodes,
                       uint32_t const *pPressedScancodes,
                       uint32_t pressedCount,
                       uint32_t const *pReleasedKeycodes,
                       uint32_t const *pReleasedScancodes,
                       uint32_t releasedCount) noexcept;

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

#endif // FOE_IMGUI_VK_RENDERER_HPP