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
#include <foe/resource/loader_base.hpp>
#include <foe/resource/material.hpp>
#include <vulkan/vulkan.h>

#include <atomic>
#include <functional>
#include <mutex>
#include <system_error>
#include <vector>

class foeShaderPool;
class foeGfxVkFragmentDescriptorPool;
class foeImageLoader;
class foeImagePool;

class FOE_RES_EXPORT foeMaterialLoader : public foeResourceLoaderBase {
  public:
    ~foeMaterialLoader();

    std::error_code initialize(foeShaderPool *pShaderPool,
                               foeImageLoader *pImageLoader,
                               foeImagePool *pImagePool,
                               foeGfxSession session,
                               std::function<foeResourceCreateInfoBase *(foeId)> importFunction,
                               std::function<void(std::function<void()>)> asynchronousJobs);
    void deinitialize() override;
    bool initialized() const noexcept;

    void processUnloadRequests();

    void requestResourceLoad(foeMaterial *pMaterial);
    void requestResourceUnload(foeMaterial *pMaterial);

    VkDescriptorSet createDescriptorSet(foeMaterial *pMaterial, uint32_t frameIndex);

  private:
    FOE_RESOURCE_NO_EXPORT void loadResource(foeMaterial *pMaterial);

    FOE_RESOURCE_NO_EXPORT foeGfxVkFragmentDescriptorPool *mGfxFragmentDescriptorPool{nullptr};

    FOE_RESOURCE_NO_EXPORT foeShaderPool *mShaderPool{nullptr};
    FOE_RESOURCE_NO_EXPORT foeImageLoader *mImageLoader{nullptr};
    FOE_RESOURCE_NO_EXPORT foeImagePool *mImagePool{nullptr};

    FOE_RESOURCE_NO_EXPORT std::function<foeResourceCreateInfoBase *(foeId)> mImportFunction;
    FOE_RESOURCE_NO_EXPORT std::function<void(std::function<void()>)> mAsyncJobs;
    FOE_RESOURCE_NO_EXPORT std::atomic_int mActiveJobs;

    FOE_RESOURCE_NO_EXPORT std::mutex mUnloadSync{};
    FOE_RESOURCE_NO_EXPORT
    std::array<std::vector<foeMaterial::Data>, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1>
        mUnloadRequestLists{};
    FOE_RESOURCE_NO_EXPORT
    std::array<std::vector<foeMaterial::Data>, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1>::iterator
        mCurrentUnloadRequests{mUnloadRequestLists.begin()};

    // VULKAN DESCRIPTORS
    FOE_RESOURCE_NO_EXPORT foeGfxSession mGfxSession;
    FOE_RESOURCE_NO_EXPORT std::array<VkDescriptorPool, FOE_GRAPHICS_MAX_BUFFERED_FRAMES>
        mDescriptorPools{};
};

#endif // FOE_RESOURCE_MATERIAL_LOADER_HPP
