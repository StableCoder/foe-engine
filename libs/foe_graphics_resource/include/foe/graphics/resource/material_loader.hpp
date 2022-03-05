/*
    Copyright (C) 2021-2022 George Cave.

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

#ifndef FOE_GRPAHICS_RESOURCE_MATERIAL_LOADER_HPP
#define FOE_GRPAHICS_RESOURCE_MATERIAL_LOADER_HPP

#include <foe/graphics/vk/fragment_descriptor_pool.hpp>

#include <foe/graphics/resource/export.h>
#include <foe/graphics/resource/material.hpp>
#include <foe/graphics/session.hpp>
#include <foe/graphics/type_defs.hpp>
#include <vulkan/vulkan.h>

#include <array>
#include <vector>

class foeShaderPool;
class foeImagePool;

struct FOE_GFX_RES_EXPORT foeMaterialCreateInfo : public foeResourceCreateInfoBase {
    ~foeMaterialCreateInfo();

    foeId fragmentShader = FOE_INVALID_ID;
    foeId image = FOE_INVALID_ID;
    bool hasRasterizationSCI;
    VkPipelineRasterizationStateCreateInfo rasterizationSCI;
    bool hasDepthStencilSCI;
    VkPipelineDepthStencilStateCreateInfo depthStencilSCI;
    bool hasColourBlendSCI;
    VkPipelineColorBlendStateCreateInfo colourBlendSCI;
    std::vector<VkPipelineColorBlendAttachmentState> colourBlendAttachments;
};

class FOE_GFX_RES_EXPORT foeMaterialLoader {
  public:
    auto initialize(foeShaderPool *pShaderPool, foeImagePool *pImagePool) -> std::error_code;
    void deinitialize();
    bool initialized() const noexcept;

    auto initializeGraphics(foeGfxSession gfxSession) -> std::error_code;
    void deinitializeGraphics();
    bool initializedGraphics() const noexcept;

    void gfxMaintenance();

    static bool canProcessCreateInfo(foeResourceCreateInfoBase *pCreateInfo);
    static void load(void *pLoader,
                     void *pResource,
                     std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
                     void (*pPostLoadFn)(void *, std::error_code));

  private:
    std::error_code createDescriptorSet(foeMaterial::Data *pMaterialData);

    static void unloadResource(void *pContext,
                               void *pResource,
                               uint32_t resourceIteration,
                               bool immediateUnload);

    void load(void *pResource,
              std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
              void (*pPostLoadFn)(void *, std::error_code));

    foeShaderPool *mShaderPool{nullptr};
    foeImagePool *mImagePool{nullptr};
    foeGfxSession mGfxSession{FOE_NULL_HANDLE};

    foeGfxVkFragmentDescriptorPool *mGfxFragmentDescriptorPool{nullptr};

    VkDescriptorPool mDescriptorPool{VK_NULL_HANDLE};

    struct LoadData {
        foeMaterial *pMaterial;
        void (*pPostLoadFn)(void *, std::error_code);
        foeMaterial::Data data;
    };

    std::mutex mLoadSync;
    std::vector<LoadData> mLoadRequests;

    struct UnloadData {
        foeMaterial *pMaterial;
        uint32_t iteration;
    };

    std::mutex mUnloadSync;
    std::vector<UnloadData> mUnloadRequests;

    size_t mDataDestroyIndex{0};
    std::array<std::vector<foeMaterial::Data>, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1>
        mDataDestroyLists{};
};

#endif // FOE_GRPAHICS_RESOURCE_MATERIAL_LOADER_HPP