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

#include <foe/imgui_renderer.hpp>

#include <GLFW/glfw3.h>
#include <foe/graphics/environment.hpp>
#include <foe/graphics/image.hpp>
#include <foe/graphics/resource_uploader.hpp>
#include <foe/graphics/upload_data.hpp>
#include <foe/wsi.hpp>
#include <imgui.h>

#include "fragment_shader.h"
#include "vertex_shader.h"

#include <array>
#include <cmath>

foeImGuiRenderer::foeImGuiRenderer() {
    ImGui::CreateContext();

    // Colour Scheme
    ImGuiStyle &style = ImGui::GetStyle();
    style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 0.0f, 0.0f, 0.1f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.8f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
    style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);

    // Dimensions
    ImGuiIO &io = ImGui::GetIO();
    io.FontGlobalScale = 1.0f;
    io.DisplayFramebufferScale = ImVec2(1.f, 1.f);

    // Keymap [GLFW]
    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_KeyPadEnter] = GLFW_KEY_KP_ENTER;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;
}

foeImGuiRenderer::~foeImGuiRenderer() { ImGui::DestroyContext(); }

VkResult foeImGuiRenderer::initialize(foeGfxEnvironment *pGfxEnvironment,
                                      VkSampleCountFlags rasterSampleFlags,
                                      VkRenderPass renderPass,
                                      uint32_t subpass) {
    if (initialized()) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkResult res;
    foeResourceUploader resUploader{};
    VkBuffer stagingBuffer{VK_NULL_HANDLE};
    VmaAllocation stagingAlloc{VK_NULL_HANDLE};
    foeUploadData uploadData{};

    VkExtent3D fontExtent;

    res = foeGfxCreateResourceUploader(pGfxEnvironment, &resUploader);
    if (res != VK_SUCCESS) {
        goto INITIALIZATION_FAILED;
    }

    { // Font
        uint8_t *pFontData;
        int imgWidth, imgHeight;
        ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pFontData, &imgWidth, &imgHeight);
        fontExtent = VkExtent3D{.width = static_cast<uint32_t>(imgWidth),
                                .height = static_cast<uint32_t>(imgHeight),
                                .depth = 1};
        VkDeviceSize uploadSize = imgWidth * imgHeight * 4;

        // Staging Buffer
        VkBufferCreateInfo bufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = uploadSize,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        };

        VmaAllocationCreateInfo allocCI{
            .usage = VMA_MEMORY_USAGE_CPU_ONLY,
        };

        res = vmaCreateBuffer(pGfxEnvironment->allocator, &bufferCI, &allocCI, &stagingBuffer,
                              &stagingAlloc, nullptr);
        if (res != VK_SUCCESS) {
            goto INITIALIZATION_FAILED;
        }

        // Map data in
        void *pData;
        res = vmaMapMemory(pGfxEnvironment->allocator, stagingAlloc, &pData);
        if (res != VK_SUCCESS) {
            goto INITIALIZATION_FAILED;
        }

        memcpy(pData, pFontData, uploadSize);

        vmaUnmapMemory(pGfxEnvironment->allocator, stagingAlloc);

        // Image
        VkImageCreateInfo imageCI{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .extent = fontExtent,
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        };

        allocCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        res = vmaCreateImage(pGfxEnvironment->allocator, &imageCI, &allocCI, &mFontImage,
                             &mFontAlloc, nullptr);
        if (res != VK_SUCCESS) {
            goto INITIALIZATION_FAILED;
        }
    }

    { // Record and submit upload commands
        VkImageSubresourceRange subresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .layerCount = 1,
        };

        VkBufferImageCopy imgCopy{
            .bufferOffset = 0,
            .imageSubresource =
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            .imageExtent = fontExtent,
        };

        res = recordImageUploadCommands(&resUploader, &subresourceRange, 1, &imgCopy, stagingBuffer,
                                        mFontImage, VK_ACCESS_SHADER_READ_BIT,
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &uploadData);
        if (res != VK_SUCCESS) {
            goto INITIALIZATION_FAILED;
        }

        res = submitUploadDataCommands(&resUploader, &uploadData);
        if (res != VK_SUCCESS) {
            goto SUBMIT_FAILED;
        }
    }

    { // Image View
        VkImageViewCreateInfo viewCI{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = mFontImage,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,
                           VK_COMPONENT_SWIZZLE_A},
            .subresourceRange =
                VkImageSubresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .levelCount = 1,
                    .layerCount = 1,
                },
        };

        res = vkCreateImageView(pGfxEnvironment->device, &viewCI, nullptr, &mFontView);
        if (res != VK_SUCCESS) {
            goto INITIALIZATION_FAILED;
        }
    }

    { // Sampler
        VkSamplerCreateInfo samplerCI{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias = 0.f,
            .anisotropyEnable = VK_FALSE,
            .maxAnisotropy = 1.0f,
            .minLod = -1000,
            .maxLod = 1000,
        };

        res = vkCreateSampler(pGfxEnvironment->device, &samplerCI, nullptr, &mFontSampler);
        if (res != VK_SUCCESS) {
            goto INITIALIZATION_FAILED;
        }
    }

    res = initializeDescriptorPool(pGfxEnvironment->device);
    if (res != VK_SUCCESS) {
        goto INITIALIZATION_FAILED;
    }

    res = initializeDescriptorSetLayout(pGfxEnvironment->device);
    if (res != VK_SUCCESS) {
        goto INITIALIZATION_FAILED;
    }

    res = initializeDescriptorSet(pGfxEnvironment->device);
    if (res != VK_SUCCESS) {
        goto INITIALIZATION_FAILED;
    }

    res = initializePipelineLayout(pGfxEnvironment->device);
    if (res != VK_SUCCESS) {
        goto INITIALIZATION_FAILED;
    }

    res = initializePipeline(pGfxEnvironment->device, rasterSampleFlags, renderPass, subpass);
    if (res != VK_SUCCESS) {
        goto INITIALIZATION_FAILED;
    }

SUBMIT_FAILED:
    if (uploadData.dstFence != VK_NULL_HANDLE) {
        VkResult fenceStatus = VK_NOT_READY;
        while (fenceStatus == VK_NOT_READY) {
            fenceStatus = vkGetFenceStatus(pGfxEnvironment->device, uploadData.dstFence);
        }
        if (fenceStatus != VK_SUCCESS) {
            res = fenceStatus;
        }
    }

INITIALIZATION_FAILED:
    uploadData.destroy(pGfxEnvironment->device);

    if (stagingBuffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(pGfxEnvironment->allocator, stagingBuffer, stagingAlloc);
    }

    foeGfxDestroyResourceUploader(&resUploader);

    if (res != VK_SUCCESS) {
        deinitialize(pGfxEnvironment);
    }

    return res;
}

void foeImGuiRenderer::deinitialize(foeGfxEnvironment *pGfxEnvironment) {
    if (mPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(pGfxEnvironment->device, mPipeline, nullptr);
        mPipeline = VK_NULL_HANDLE;
    }

    if (mPipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(pGfxEnvironment->device, mPipelineLayout, nullptr);
        mPipelineLayout = VK_NULL_HANDLE;
    }

    if (mDescriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(pGfxEnvironment->device, mDescriptorPool, nullptr);
        mDescriptorPool = VK_NULL_HANDLE;
    }

    if (mDescriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(pGfxEnvironment->device, mDescriptorSetLayout, nullptr);
        mDescriptorSetLayout = VK_NULL_HANDLE;
    }

    if (mFontSampler != VK_NULL_HANDLE) {
        vkDestroySampler(pGfxEnvironment->device, mFontSampler, nullptr);
        mFontSampler = VK_NULL_HANDLE;
    }

    if (mFontView != VK_NULL_HANDLE) {
        vkDestroyImageView(pGfxEnvironment->device, mFontView, nullptr);
        mFontView = VK_NULL_HANDLE;
    }

    if (mFontImage != VK_NULL_HANDLE) {
        vmaDestroyImage(pGfxEnvironment->allocator, mFontImage, mFontAlloc);
        mFontImage = VK_NULL_HANDLE;
    }

    for (auto &it : mDrawBuffers) {
        if (it.vertices.buffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(pGfxEnvironment->allocator, it.vertices.buffer, it.vertices.alloc);
        }

        if (it.indices.buffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(pGfxEnvironment->allocator, it.indices.buffer, it.indices.alloc);
        }
    }
    mDrawBuffers.clear();
}

bool foeImGuiRenderer::initialized() const noexcept { return mPipeline != VK_NULL_HANDLE; }

void foeImGuiRenderer::newFrame() { ImGui::NewFrame(); }

void foeImGuiRenderer::endFrame() { ImGui::Render(); }

VkResult foeImGuiRenderer::update(VmaAllocator allocator, uint32_t frameIndex) {
    VkResult res;

    if (mDrawBuffers.size() <= frameIndex) {
        mDrawBuffers.resize(frameIndex + 1);
    }
    BufferSet &vertices = mDrawBuffers[frameIndex].vertices;
    BufferSet &indices = mDrawBuffers[frameIndex].indices;

    ImDrawData const *pDrawData = ImGui::GetDrawData();
    if (pDrawData == nullptr) {
        return VK_SUCCESS;
    }

    VkDeviceSize vertexBufferSize = pDrawData->TotalVtxCount * sizeof(ImDrawVert);
    VkDeviceSize indexBufferSize = pDrawData->TotalIdxCount * sizeof(ImDrawIdx);

    // Update buffers only if vertex or indice count has been changed compared to the current buffer
    // size
    if (vertexBufferSize == 0 || indexBufferSize == 0)
        return VK_SUCCESS;

    // Vertex Buffer
    if (vertices.buffer != VK_NULL_HANDLE || vertices.count != pDrawData->TotalVtxCount) {
        if (vertices.buffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(allocator, vertices.buffer, vertices.alloc);
        }

        VkBufferCreateInfo bufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = vertexBufferSize,
            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        };

        VmaAllocationCreateInfo allocCI{
            .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
        };

        res = vmaCreateBuffer(allocator, &bufferCI, &allocCI, &vertices.buffer, &vertices.alloc,
                              nullptr);
        if (res != VK_SUCCESS) {
            return res;
        }

        vertices.count = pDrawData->TotalVtxCount;
    }

    // Index Buffer
    if (indices.buffer != VK_NULL_HANDLE || indices.count != pDrawData->TotalIdxCount) {
        if (indices.buffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(allocator, indices.buffer, indices.alloc);
        }

        VkBufferCreateInfo bufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = indexBufferSize,
            .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        };

        VmaAllocationCreateInfo allocCI{
            .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
        };

        res = vmaCreateBuffer(allocator, &bufferCI, &allocCI, &indices.buffer, &indices.alloc,
                              nullptr);
        if (res != VK_SUCCESS) {
            return res;
        }

        indices.count = pDrawData->TotalIdxCount;
    }

    // Copy data for use
    ImDrawVert *pVertexData;
    ImDrawIdx *pIndexData;

    res = vmaMapMemory(allocator, vertices.alloc, reinterpret_cast<void **>(&pVertexData));
    if (res != VK_SUCCESS) {
        return res;
    }

    res = vmaMapMemory(allocator, indices.alloc, reinterpret_cast<void **>(&pIndexData));
    if (res != VK_SUCCESS) {
        return res;
    }

    for (int i = 0; i < pDrawData->CmdListsCount; ++i) {
        ImDrawList const *cmdList = pDrawData->CmdLists[i];

        memcpy(pVertexData, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(pIndexData, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));

        pVertexData += cmdList->VtxBuffer.Size;
        pIndexData += cmdList->VtxBuffer.Size;
    }

    vmaUnmapMemory(allocator, indices.alloc);
    vmaUnmapMemory(allocator, vertices.alloc);

    return VK_SUCCESS;
}

void foeImGuiRenderer::draw(VkCommandBuffer commandBuffer, uint32_t frameIndex) {
    BufferSet &vertices = mDrawBuffers[frameIndex].vertices;
    BufferSet &indices = mDrawBuffers[frameIndex].indices;

    ImDrawData const *pDrawData = ImGui::GetDrawData();

    if (pDrawData == nullptr || pDrawData->CmdListsCount == 0) {
        return;
    }

    ImGuiIO const &io = ImGui::GetIO();

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1,
                            &mDescriptorSet, 0, nullptr);

    float pushConstants[4] = {
        2.f / io.DisplaySize.x,
        2.f / io.DisplaySize.y,
        -1.f,
        -1.f,
    };
    vkCmdPushConstants(commandBuffer, mPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                       sizeof(pushConstants), &pushConstants);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertices.buffer, &offset);
    vkCmdBindIndexBuffer(commandBuffer, indices.buffer, offset,
                         sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);

    int32_t vertexOffset = 0;
    int32_t indexOffset = 0;
    for (int32_t i = 0; i < pDrawData->CmdListsCount; ++i) {
        ImDrawList const *pCmdList = pDrawData->CmdLists[i];

        for (int32_t j = 0; j < pCmdList->CmdBuffer.Size; ++j) {
            ImDrawCmd const *pDrawCmd = &pCmdList->CmdBuffer[j];

            VkRect2D scissorRect{
                .offset =
                    {
                        .x = std::max((int32_t)(pDrawCmd->ClipRect.x), 0),
                        .y = std::max((int32_t)(pDrawCmd->ClipRect.y), 0),
                    },
                .extent =
                    {
                        .width = (uint32_t)(pDrawCmd->ClipRect.z - pDrawCmd->ClipRect.x),
                        .height = (uint32_t)(pDrawCmd->ClipRect.w - pDrawCmd->ClipRect.y),
                    },
            };

            vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
            vkCmdDrawIndexed(commandBuffer, pDrawCmd->ElemCount, 1, indexOffset, vertexOffset, 0);
            indexOffset += pDrawCmd->ElemCount;
        }
        vertexOffset += pCmdList->VtxBuffer.Size;
    }
}

void foeImGuiRenderer::resize(float width, float height) {
    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2(width, height);
}

void foeImGuiRenderer::rescale(float xScale, float yScale) {
    ImGuiIO &io = ImGui::GetIO();
    io.DisplayFramebufferScale = ImVec2{xScale, yScale};
}

bool foeImGuiRenderer::wantCaptureMouse() const noexcept { return ImGui::GetIO().WantCaptureMouse; }

void foeImGuiRenderer::mouseInput(foeMouse const *pMouse) noexcept {
    ImGuiIO &io = ImGui::GetIO();

    // Buttons down
    std::fill_n(io.MouseDown, IM_ARRAYSIZE(io.MouseDown), false);
    for (auto const &it : pMouse->downButtons) {
        io.MouseDown[it] = true;
    }

    // Mouse pos
    io.MousePos.x = pMouse->position.x;
    io.MousePos.y = pMouse->position.y;

    // Mouse wheel
    io.MouseWheelH += pMouse->scroll.x;
    io.MouseWheel += pMouse->scroll.y;
}

bool foeImGuiRenderer::wantCaptureKeyboard() const noexcept {
    return ImGui::GetIO().WantCaptureKeyboard;
}

void foeImGuiRenderer::keyboardInput(foeKeyboard const *pKeyboard) noexcept {
    ImGuiIO &io = ImGui::GetIO();

    // Down keys
    std::fill_n(io.KeysDown, IM_ARRAYSIZE(io.KeysDown), false);
    for (auto const &it : pKeyboard->downKeys) {
        io.KeysDown[it] = true;
    }

    // Character
    if (pKeyboard->unicodeChar != 0) {
        io.AddInputCharacter(pKeyboard->unicodeChar);
    }
}

VkResult foeImGuiRenderer::initializeDescriptorPool(VkDevice device) {
    VkDescriptorPoolSize poolSize{
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
    };

    VkDescriptorPoolCreateInfo poolCI{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize,
    };

    return vkCreateDescriptorPool(device, &poolCI, nullptr, &mDescriptorPool);
}

VkResult foeImGuiRenderer::initializeDescriptorSetLayout(VkDevice device) {
    VkDescriptorSetLayoutBinding binding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    VkDescriptorSetLayoutCreateInfo layoutCI{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &binding,
    };

    return vkCreateDescriptorSetLayout(device, &layoutCI, nullptr, &mDescriptorSetLayout);
}

VkResult foeImGuiRenderer::initializeDescriptorSet(VkDevice device) {
    VkDescriptorSetAllocateInfo setAI{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = mDescriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &mDescriptorSetLayout,
    };

    VkResult res = vkAllocateDescriptorSets(device, &setAI, &mDescriptorSet);
    if (res != VK_SUCCESS) {
        return res;
    }

    // Write the set
    VkDescriptorImageInfo imageInfo{
        .sampler = mFontSampler,
        .imageView = mFontView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    VkWriteDescriptorSet writeSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = mDescriptorSet,
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &imageInfo,
    };

    vkUpdateDescriptorSets(device, 1, &writeSet, 0, nullptr);

    return res;
}

VkResult foeImGuiRenderer::initializePipelineLayout(VkDevice device) {
    VkPushConstantRange pushConstantRange{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(float) * 4,
    };

    VkPipelineLayoutCreateInfo layoutCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &mDescriptorSetLayout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange,
    };

    return vkCreatePipelineLayout(device, &layoutCI, nullptr, &mPipelineLayout);
}

VkResult foeImGuiRenderer::initializePipeline(VkDevice device,
                                              VkSampleCountFlags rasterSampleFlags,
                                              VkRenderPass renderPass,
                                              uint32_t subpass) {
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    };

    VkPipelineRasterizationStateCreateInfo rasterizationStateCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .lineWidth = 1.0f,
    };

    // Enable Blending
    VkPipelineColorBlendAttachmentState blendAttachmentState{
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    VkPipelineColorBlendStateCreateInfo colorBlendStateCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &blendAttachmentState,
    };

    VkPipelineDepthStencilStateCreateInfo depthStencilStateCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_FALSE,
        .depthWriteEnable = VK_FALSE,
        .depthCompareOp = VK_COMPARE_OP_ALWAYS,
        .depthBoundsTestEnable = VK_FALSE,
        .front =
            {
                .compareOp = VK_COMPARE_OP_ALWAYS,
            },
        .back =
            {
                .compareOp = VK_COMPARE_OP_ALWAYS,
            },
    };

    VkPipelineViewportStateCreateInfo viewportStateCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .flags = 0,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    VkPipelineMultisampleStateCreateInfo multisampleStateCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .flags = 0,
        .rasterizationSamples = static_cast<VkSampleCountFlagBits>(rasterSampleFlags),
    };

    std::array<VkDynamicState, 2> dynamicStateEnables{
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo dynamicStateCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = dynamicStateEnables.size(),
        .pDynamicStates = dynamicStateEnables.data(),
    };

    std::array<VkVertexInputBindingDescription, 1> vertexInputBindings{
        VkVertexInputBindingDescription{
            .binding = 0,
            .stride = sizeof(ImDrawVert),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        },
    };

    std::array<VkVertexInputAttributeDescription, 3> vertexInputAttributes{
        VkVertexInputAttributeDescription{
            // Location 0, Position
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(ImDrawVert, pos),
        },
        VkVertexInputAttributeDescription{
            // Location 1, UV
            .location = 1,
            .binding = 0,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(ImDrawVert, uv),
        },
        VkVertexInputAttributeDescription{
            // Location 2, Color
            .location = 2,
            .binding = 0,
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .offset = offsetof(ImDrawVert, col),
        },
    };

    VkPipelineVertexInputStateCreateInfo vertexInputStateCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = vertexInputBindings.size(),
        .pVertexBindingDescriptions = vertexInputBindings.data(),
        .vertexAttributeDescriptionCount = vertexInputAttributes.size(),
        .pVertexAttributeDescriptions = vertexInputAttributes.data(),
    };

    VkShaderModuleCreateInfo shaderCI{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    };

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{
        VkPipelineShaderStageCreateInfo{
            // Vertex Shader
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .pName = "main",
        },
        VkPipelineShaderStageCreateInfo{
            // Fragment Shader
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pName = "main",

        },
    };

    // Vertex Shader
    shaderCI.codeSize = sizeof(vertexShader);
    shaderCI.pCode = vertexShader;
    vkCreateShaderModule(device, &shaderCI, nullptr, &shaderStages[0].module);

    // Fragment Shader
    shaderCI.codeSize = sizeof(fragmentShader);
    shaderCI.pCode = fragmentShader;
    vkCreateShaderModule(device, &shaderCI, nullptr, &shaderStages[1].module);

    // Pipeline
    VkGraphicsPipelineCreateInfo graphicsPipelineCI{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = shaderStages.size(),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputStateCI,
        .pInputAssemblyState = &inputAssemblyCI,
        .pViewportState = &viewportStateCI,
        .pRasterizationState = &rasterizationStateCI,
        .pMultisampleState = &multisampleStateCI,
        .pDepthStencilState = &depthStencilStateCI,
        .pColorBlendState = &colorBlendStateCI,
        .pDynamicState = &dynamicStateCI,
        .layout = mPipelineLayout,
        .renderPass = renderPass,
        .subpass = 0,
    };

    VkResult res = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCI,
                                             nullptr, &mPipeline);

    vkDestroyShaderModule(device, shaderStages[0].module, nullptr);
    vkDestroyShaderModule(device, shaderStages[1].module, nullptr);

    return res;
}