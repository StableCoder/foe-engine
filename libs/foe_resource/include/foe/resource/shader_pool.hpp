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

#ifndef FOE_RESOURCE_SHADER_POOL_HPP
#define FOE_RESOURCE_SHADER_POOL_HPP

#include <foe/ecs/id.hpp>
#include <foe/resource/export.h>
#include <foe/resource/pool_base.hpp>

#include <shared_mutex>
#include <vector>

class foeShader;

class foeShaderPool : public foeResourcePoolBase {
  public:
    FOE_RES_EXPORT ~foeShaderPool();

    FOE_RES_EXPORT bool add(foeShader *pShader);
    FOE_RES_EXPORT foeShader *find(foeId resource);

    FOE_RES_EXPORT void unloadAll();

    std::vector<foeShader *> const &getEntries() const noexcept { return mShaders; }

    FOE_RES_EXPORT auto getDataVector() { return mShaders; }

  private:
    std::shared_mutex mSync;
    std::vector<foeShader *> mShaders;
};

#endif // FOE_RESOURCE_SHADER_POOL_HPP