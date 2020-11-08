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

#ifndef FOE_SHADER_MANAGER_HPP
#define FOE_SHADER_MANAGER_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/shader.hpp>
#include <vulkan/vulkan.h>

#include <mutex>
#include <string_view>
#include <vector>

class foeShaderPool {
  public:
    FOE_GFX_EXPORT VkResult initialize(VkDevice device);
    FOE_GFX_EXPORT void deinitialize();

    FOE_GFX_EXPORT foeShader *create(std::string_view name);
    FOE_GFX_EXPORT foeShader *find(std::string_view name);

  private:
    friend struct foeShader;

    void load(foeShader *shader);
    void unload(foeShader *shader);

    VkDevice mDevice{VK_NULL_HANDLE};

    std::mutex mSync;
    std::vector<foeShader *> mShaders;
};

#endif // FOE_SHADER_MANAGER_HPP