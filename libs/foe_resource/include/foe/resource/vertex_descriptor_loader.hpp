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
#include <foe/simulation/core/loader.hpp>

#include <atomic>
#include <functional>
#include <mutex>
#include <system_error>
#include <vector>

class foeShaderPool;

class FOE_RES_EXPORT foeVertexDescriptorLoader : public foeResourceLoaderBase {
  public:
    ~foeVertexDescriptorLoader();

    std::error_code initialize(foeShaderPool *pShaderPool,
                               std::function<foeResourceCreateInfoBase *(foeId)> importFunction,
                               std::function<void(std::function<void()>)> asynchronousJobs);
    void deinitialize();
    bool initialized() const noexcept;

    void processUnloadRequests();

    void requestResourceLoad(foeVertexDescriptor *pVertexDescriptor);
    void requestResourceUnload(foeVertexDescriptor *pVertexDescriptor);

  private:
    FOE_RESOURCE_NO_EXPORT void loadResource(foeVertexDescriptor *pVertexDescriptor);

    FOE_RESOURCE_NO_EXPORT foeShaderPool *mShaderPool{nullptr};

    FOE_RESOURCE_NO_EXPORT std::function<foeResourceCreateInfoBase *(foeId)> mImportFunction;
    FOE_RESOURCE_NO_EXPORT std::function<void(std::function<void()>)> mAsyncJobs;
    FOE_RESOURCE_NO_EXPORT std::atomic_int mActiveJobs;

    FOE_RESOURCE_NO_EXPORT std::mutex mUnloadSync{};
    FOE_RESOURCE_NO_EXPORT
    std::array<std::vector<foeVertexDescriptor::Data>, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1>
        mUnloadRequestLists{};
    FOE_RESOURCE_NO_EXPORT std::array<std::vector<foeVertexDescriptor::Data>,
                                      FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1>::iterator
        mCurrentUnloadRequests{mUnloadRequestLists.begin()};
};

#endif // FOE_RESOURCE_VERTEX_DESCRIPTOR_LOADER_HPP