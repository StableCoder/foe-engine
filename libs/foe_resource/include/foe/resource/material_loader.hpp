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

#ifndef FOE_RESOURCE_MATERIAL_LOADER_HPP
#define FOE_RESOURCE_MATERIAL_LOADER_HPP

#include <foe/graphics/session.hpp>
#include <foe/graphics/type_defs.hpp>
#include <foe/resource/export.h>
#include <foe/resource/material.hpp>
#include <vulkan/vulkan.h>

#include <atomic>
#include <functional>
#include <mutex>
#include <system_error>
#include <vector>

class foeShaderLoader;
class foeShaderPool;
class foeGfxVkFragmentDescriptorPool;
class foeImageLoader;
class foeImagePool;

class foeMaterialLoader {
  public:
    FOE_RES_EXPORT ~foeMaterialLoader();

    FOE_RES_EXPORT std::error_code initialize(
        foeShaderLoader *pShaderLoader,
        foeShaderPool *pShaderPool,
        foeGfxVkFragmentDescriptorPool *pGfxFragmentDescriptorPool,
        foeImageLoader *pImageLoader,
        foeImagePool *pImagePool,
        foeGfxSession session,
        std::function<bool(std::string_view, foeMaterialCreateInfo &)> importFunction,
        std::function<void(std::function<void()>)> asynchronousJobs);
    FOE_RES_EXPORT void deinitialize();
    FOE_RES_EXPORT bool initialized() const noexcept;

    FOE_RES_EXPORT void processUnloadRequests();

    FOE_RES_EXPORT void requestResourceLoad(foeMaterial *pMaterial);
    FOE_RES_EXPORT void requestResourceUnload(foeMaterial *pMaterial);

    FOE_RES_EXPORT VkDescriptorSet createDescriptorSet(foeMaterial *pMaterial, uint32_t frameIndex);

  private:
    void loadResource(foeMaterial *pMaterial);

    foeShaderLoader *mShaderLoader{nullptr};
    foeShaderPool *mShaderPool{nullptr};
    foeGfxVkFragmentDescriptorPool *mGfxFragmentDescriptorPool{nullptr};
    foeImageLoader *mImageLoader{nullptr};
    foeImagePool *mImagePool{nullptr};

    std::function<bool(std::string_view, foeMaterialCreateInfo &)> mImportFunction;
    std::function<void(std::function<void()>)> mAsyncJobs;
    std::atomic_int mActiveJobs;

    std::mutex mUnloadSync{};
    std::array<std::vector<foeMaterial::Data>, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1>
        mUnloadRequestLists{};
    std::vector<foeMaterial::Data> *mCurrentUnloadRequests{&mUnloadRequestLists[0]};

    // VULKAN DESCRIPTORS
    foeGfxSession mGfxSession;
    std::array<VkDescriptorPool, FOE_GRAPHICS_MAX_BUFFERED_FRAMES> mDescriptorPools{};
};

#endif // FOE_RESOURCE_MATERIAL_LOADER_HPP
