/*
    Copyright (C) 2020 George Cave.

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

#ifndef FOE_GRAPHICS_SHADER_HPP
#define FOE_GRAPHICS_SHADER_HPP

#include <foe/graphics/builtin_descriptor_sets.hpp>
#include <foe/graphics/export.h>
#include <foe/graphics/load_state.hpp>
#include <vulkan/vulkan.h>

#include <atomic>
#include <cstddef>
#include <string>

class foeShaderPool;

class foeShader {
  public:
    std::string name;
    foeShaderPool *pManager;

    foeBuiltinDescriptorSetLayoutFlags builtinSetLayouts = 0;

    VkShaderModule module{VK_NULL_HANDLE};
    VkDescriptorSetLayout layout{VK_NULL_HANDLE};
    VkPushConstantRange pushConstantRange{};

    FOE_GFX_EXPORT void incrementRefCount();
    FOE_GFX_EXPORT void decrementRefCount();
    FOE_GFX_EXPORT auto getRefCount() const noexcept -> size_t;

    FOE_GFX_EXPORT void incrementUseCount();
    FOE_GFX_EXPORT void decrementUseCount();
    FOE_GFX_EXPORT auto getUseCount() const noexcept -> size_t;

  private:
    friend class foeShaderPool;

    std::atomic<LoadState> loadState{LoadState::Unloaded};
    std::atomic<size_t> refCount{0};
    std::atomic<size_t> useCount{0};
};

#endif // FOE_GRAPHICS_SHADER_HPP