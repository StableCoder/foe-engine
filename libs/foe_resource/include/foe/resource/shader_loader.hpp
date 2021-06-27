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

#ifndef FOE_RESOURCE_SHADER_LOADER_HPP
#define FOE_RESOURCE_SHADER_LOADER_HPP

#include <foe/graphics/session.hpp>
#include <foe/graphics/type_defs.hpp>
#include <foe/resource/export.h>
#include <foe/resource/shader.hpp>
#include <foe/simulation/core/loader.hpp>
#include <vulkan/vulkan.h>

#include <atomic>
#include <filesystem>
#include <functional>
#include <mutex>
#include <system_error>
#include <vector>

class FOE_RES_EXPORT foeShaderLoader : public foeResourceLoaderBase {
  public:
    ~foeShaderLoader();

    std::error_code initialize(
        foeGfxSession gfxSession,
        std::function<foeResourceCreateInfoBase *(foeId)> importFunction,
        std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn,
        std::function<void(std::function<void()>)> asynchronousJobs);
    void deinitialize();
    bool initialized() const noexcept;

    void processUnloadRequests();

    void requestResourceLoad(foeShader *pShader);
    void requestResourceUnload(foeShader *pShader);

  private:
    FOE_RESOURCE_NO_EXPORT void loadResource(foeShader *pShader);

    FOE_RESOURCE_NO_EXPORT foeGfxSession mGfxSession{FOE_NULL_HANDLE};

    FOE_RESOURCE_NO_EXPORT std::function<foeResourceCreateInfoBase *(foeId)> mImportFunction;
    FOE_RESOURCE_NO_EXPORT std::function<std::filesystem::path(std::filesystem::path)>
        mExternalFileSearchFn;
    FOE_RESOURCE_NO_EXPORT std::function<void(std::function<void()>)> mAsyncJobs;
    FOE_RESOURCE_NO_EXPORT std::atomic_int mActiveJobs;

    FOE_RESOURCE_NO_EXPORT std::mutex mUnloadSync{};
    FOE_RESOURCE_NO_EXPORT
    std::array<std::vector<foeShader::Data>, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1>
        mUnloadRequestLists{};
    FOE_RESOURCE_NO_EXPORT
    std::array<std::vector<foeShader::Data>, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1>::iterator
        mCurrentUnloadRequests{mUnloadRequestLists.begin()};
};

#endif // FOE_RESOURCE_SHADER_LOADER_HPP