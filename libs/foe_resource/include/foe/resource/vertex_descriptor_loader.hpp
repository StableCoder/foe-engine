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

#ifndef FOE_RESOURCE_VERTEX_DESCRIPTOR_LOADER_HPP
#define FOE_RESOURCE_VERTEX_DESCRIPTOR_LOADER_HPP

#include <foe/graphics/type_defs.hpp>
#include <foe/resource/export.h>
#include <foe/resource/vertex_descriptor.hpp>

#include <atomic>
#include <functional>
#include <mutex>
#include <system_error>
#include <vector>

class foeShaderLoader;
class foeShaderPool;

class foeVertexDescriptorLoader {
  public:
    FOE_RES_EXPORT ~foeVertexDescriptorLoader();

    FOE_RES_EXPORT std::error_code initialize(
        foeShaderLoader *pShaderLoader,
        foeShaderPool *pShaderPool,
        std::function<void(std::function<void()>)> asynchronousJobs);
    FOE_RES_EXPORT void deinitialize();
    FOE_RES_EXPORT bool initialized() const noexcept;

    FOE_RES_EXPORT void processUnloadRequests();

    FOE_RES_EXPORT void requestResourceLoad(foeVertexDescriptor *pVertexDescriptor);
    FOE_RES_EXPORT void requestResourceUnload(foeVertexDescriptor *pVertexDescriptor);

  private:
    void loadResource(foeVertexDescriptor *pVertexDescriptor);

    foeShaderLoader *mShaderLoader{nullptr};
    foeShaderPool *mShaderPool{nullptr};

    std::function<void(std::function<void()>)> mAsyncJobs;
    std::atomic_int mActiveJobs;

    std::mutex mUnloadSync{};
    std::array<std::vector<foeVertexDescriptor::Data>, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1>
        mUnloadRequestLists{};
    std::vector<foeVertexDescriptor::Data> *mCurrentUnloadRequests{&mUnloadRequestLists[0]};
};

#endif // FOE_RESOURCE_VERTEX_DESCRIPTOR_LOADER_HPP