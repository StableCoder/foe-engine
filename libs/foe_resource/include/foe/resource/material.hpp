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

#ifndef FOE_RESOURCE_MATERIAL_HPP
#define FOE_RESOURCE_MATERIAL_HPP

#include <foe/ecs/id.hpp>
#include <foe/graphics/vk/fragment_descriptor.hpp>
#include <foe/resource/create_info_base.hpp>
#include <foe/resource/export.h>
#include <foe/resource/load_state.hpp>

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class foeFragmentDescriptor;
class foeShader;
class foeImage;
class foeMaterialLoader;

struct foeMaterialCreateInfo : public foeResourceCreateInfoBase {
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

struct foeMaterial {
  public:
    FOE_RES_EXPORT foeMaterial(foeId id, foeMaterialLoader *pLoader);
    FOE_RES_EXPORT ~foeMaterial();

    FOE_RES_EXPORT foeId getID() const noexcept;
    FOE_RES_EXPORT foeResourceLoadState getLoadState() const noexcept;

    FOE_RES_EXPORT int incrementRefCount() noexcept;
    FOE_RES_EXPORT int decrementRefCount() noexcept;
    FOE_RES_EXPORT int getRefCount() const noexcept;

    FOE_RES_EXPORT int incrementUseCount() noexcept;
    FOE_RES_EXPORT int decrementUseCount() noexcept;
    FOE_RES_EXPORT int getUseCount() const noexcept;

    FOE_RES_EXPORT void requestLoad();
    FOE_RES_EXPORT void requestUnload();

    // Specializations
    FOE_RES_EXPORT foeShader *getFragmentShader() const noexcept;
    FOE_RES_EXPORT foeGfxVkFragmentDescriptor *getGfxFragmentDescriptor() const noexcept;
    FOE_RES_EXPORT foeImage *getImage() const noexcept;

    FOE_RES_EXPORT VkDescriptorSet getVkDescriptorSet(uint32_t frameIndex);

  private:
    friend foeMaterialLoader;

    struct SubResources {
        // For the FragmentDescriptor
        foeShader *pFragmentShader{nullptr};
        // For the Material
        foeImage *pImage{nullptr};

        SubResources() = default;
        FOE_RES_EXPORT ~SubResources();

        SubResources(SubResources const &) = delete;
        SubResources &operator=(SubResources const &) = delete;

        SubResources(SubResources &&);
        SubResources &operator=(SubResources &&);

        void reset();
        foeResourceLoadState getWorstSubresourceState() const noexcept;
    };

    // General
    foeId id;
    std::atomic<foeResourceLoadState> loadState{foeResourceLoadState::Unloaded};
    std::atomic_int refCount{0};
    std::atomic_int useCount{0};

    // Specialization
    foeMaterialLoader *pLoader;

  public:
    std::mutex dataWriteLock{};
    SubResources loadingSubResources;
    std::unique_ptr<foeMaterialCreateInfo> createInfo;
    struct Data {
        SubResources subResources;
        foeGfxVkFragmentDescriptor *pGfxFragDescriptor;
        VkDescriptorSet materialDescriptorSet;
    };
    Data data{};
};

#endif // FOE_RESOURCE_MATERIAL_HPP