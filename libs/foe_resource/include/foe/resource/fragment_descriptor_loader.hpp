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

#ifndef FOE_RESOURCES_FRAGMENT_DESCRIPTOR_LOADER_HPP
#define FOE_RESOURCES_FRAGMENT_DESCRIPTOR_LOADER_HPP

#include <foe/graphics/type_defs.hpp>
#include <foe/graphics/vk/fragment_descriptor_pool.hpp>
#include <foe/resource/export.h>
#include <foe/resource/fragment_descriptor.hpp>

#include <atomic>
#include <functional>
#include <mutex>
#include <system_error>
#include <vector>

class foeShaderLoader;
class foeShaderPool;

class foeFragmentDescriptorLoader {
  public:
    FOE_RES_EXPORT ~foeFragmentDescriptorLoader();

    FOE_RES_EXPORT std::error_code initialize(
        foeGfxVkFragmentDescriptorPool *pFragPool,
        foeShaderLoader *pShaderLoader,
        foeShaderPool *pShaderPool,
        std::function<void(std::function<void()>)> asynchronousJobs);
    FOE_RES_EXPORT void deinitialize();
    FOE_RES_EXPORT bool initialized() const noexcept;

    FOE_RES_EXPORT void processLoadRequests();
    FOE_RES_EXPORT void processUnloadRequests();

    FOE_RES_EXPORT void requestResourceLoad(foeFragmentDescriptor *pFragDescriptor);
    FOE_RES_EXPORT void requestResourceUnload(foeFragmentDescriptor *pFragDescriptor);

  private:
    void loadResource(foeFragmentDescriptor *pFragDescriptor);

    foeGfxVkFragmentDescriptorPool *mFragPool{nullptr};
    foeShaderLoader *mShaderLoader{nullptr};
    foeShaderPool *mShaderPool{nullptr};

    std::function<void(std::function<void()>)> mAsyncJobs;
    std::atomic_int mActiveJobs;

    std::mutex mLoadSync{};
    std::vector<foeFragmentDescriptor *> mLoadRequests{};

    std::mutex mUnloadSync{};
    std::array<std::vector<foeFragmentDescriptor::Data>, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1>
        mUnloadRequestLists{};
    std::vector<foeFragmentDescriptor::Data> *mCurrentUnloadRequests{&mUnloadRequestLists[0]};
};

#endif // FOE_RESOURCES_FRAGMENT_DESCRIPTOR_LOADER_HPP