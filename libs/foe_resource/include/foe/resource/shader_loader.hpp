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
#include <vulkan/vulkan.h>

#include <atomic>
#include <functional>
#include <mutex>
#include <system_error>
#include <vector>

class foeShaderLoader {
  public:
    FOE_RES_EXPORT ~foeShaderLoader();

    FOE_RES_EXPORT std::error_code initialize(
        foeGfxSession gfxSession,
        std::function<bool(std::string_view, foeShaderCreateInfo &)> importFunction,
        std::function<void(std::function<void()>)> asynchronousJobs);
    FOE_RES_EXPORT void deinitialize();
    FOE_RES_EXPORT bool initialized() const noexcept;

    FOE_RES_EXPORT void processUnloadRequests();

    FOE_RES_EXPORT void requestResourceLoad(foeShader *pShader);
    FOE_RES_EXPORT void requestResourceUnload(foeShader *pShader);

  private:
    void loadResource(foeShader *pShader);

    foeGfxSession mGfxSession{FOE_NULL_HANDLE};

    std::function<bool(std::string_view, foeShaderCreateInfo &)> mImportFunction;
    std::function<void(std::function<void()>)> mAsyncJobs;
    std::atomic_int mActiveJobs;

    std::mutex mUnloadSync{};
    std::array<std::vector<foeShader::Data>, FOE_GRAPHICS_MAX_BUFFERED_FRAMES + 1>
        mUnloadRequestLists{};
    std::vector<foeShader::Data> *mCurrentUnloadRequests{&mUnloadRequestLists[0]};
};

#endif // FOE_RESOURCE_SHADER_LOADER_HPP