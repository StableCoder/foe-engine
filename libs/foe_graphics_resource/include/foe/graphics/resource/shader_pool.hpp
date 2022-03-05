/*
    Copyright (C) 2021-2022 George Cave.

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

#ifndef FOE_GRAPHICS_RESOURCE_SHADER_POOL_HPP
#define FOE_GRAPHICS_RESOURCE_SHADER_POOL_HPP

#include <foe/graphics/resource/export.h>
#include <foe/simulation/core/resource_fns.hpp>

#include <shared_mutex>
#include <vector>

struct foeShader;

class FOE_GFX_RES_EXPORT foeShaderPool {
  public:
    foeShaderPool(foeResourceFns const &resourceFns);
    ~foeShaderPool();

    foeShader *add(foeResourceID resource);
    foeShader *findOrAdd(foeResourceID resource);
    foeShader *find(foeId id);

    void setAsyncTaskFn(std::function<void(std::function<void()>)> asyncTaskFn);

    void unloadAll();

    auto getDataVector() { return mResources; }

  private:
    FOE_GRAPHICS_RESOURCE_NO_EXPORT foeResourceFns mResourceFns;
    FOE_GRAPHICS_RESOURCE_NO_EXPORT std::shared_mutex mSync;
    FOE_GRAPHICS_RESOURCE_NO_EXPORT std::vector<foeShader *> mResources;
};

#endif // FOE_GRAPHICS_RESOURCE_SHADER_POOL_HPP