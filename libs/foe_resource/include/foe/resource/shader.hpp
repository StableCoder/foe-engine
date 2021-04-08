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

#ifndef FOE_RESOURCE_SHADER_HPP
#define FOE_RESOURCE_SHADER_HPP

#include <foe/ecs/resource_id.hpp>
#include <foe/graphics/shader.hpp>
#include <foe/resource/create_info_base.hpp>
#include <foe/resource/export.h>
#include <foe/resource/load_state.hpp>
#include <vulkan/vulkan.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class foeShaderLoader;

struct foeShaderCreateInfo : public foeResourceCreateInfoBase {
    std::string shaderCodeFile;
    foeBuiltinDescriptorSetLayoutFlags builtinSetLayouts;
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI;
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
    VkPushConstantRange pushConstantRange;
};

class foeShader {
  public:
    FOE_RES_EXPORT foeShader(foeResourceID id, foeShaderLoader *pLoader);
    FOE_RES_EXPORT ~foeShader();

    FOE_RES_EXPORT foeResourceID getID() const noexcept;
    FOE_RES_EXPORT foeResourceLoadState getLoadState() const noexcept;

    FOE_RES_EXPORT int incrementRefCount() noexcept;
    FOE_RES_EXPORT int decrementRefCount() noexcept;
    FOE_RES_EXPORT int getRefCount() const noexcept;

    FOE_RES_EXPORT int incrementUseCount() noexcept;
    FOE_RES_EXPORT int decrementUseCount() noexcept;
    FOE_RES_EXPORT int getUseCount() const noexcept;

    FOE_RES_EXPORT foeGfxShader getShader() const noexcept;

    FOE_RES_EXPORT void requestLoad();
    FOE_RES_EXPORT void requestUnload();

  private:
    friend foeShaderLoader;

    // General
    foeResourceID id;
    std::atomic<foeResourceLoadState> loadState{foeResourceLoadState::Unloaded};
    std::atomic_int refCount{0};
    std::atomic_int useCount{0};

    // Specialization
    foeShaderLoader *const pLoader;

    std::mutex dataWriteLock{};
    struct Data {
        foeGfxShader shader;
    };
    Data data{};
};

#endif // FOE_RESOURCE_SHADER_HPP