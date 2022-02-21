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

#ifndef FOE_RESOURCE_ARMATURE_POOL_HPP
#define FOE_RESOURCE_ARMATURE_POOL_HPP

#include <foe/resource/export.h>
#include <foe/simulation/core/pool.hpp>
#include <foe/simulation/core/resource_fns.hpp>

#include <shared_mutex>
#include <vector>

struct foeArmature;

class FOE_RES_EXPORT foeArmaturePool : public foeResourcePoolBase {
  public:
    foeArmaturePool(foeResourceFns const &resourceFns);
    ~foeArmaturePool();

    foeArmature *add(foeResourceID resource);
    foeArmature *findOrAdd(foeResourceID resource);
    foeArmature *find(foeId id);

    void setAsyncTaskFn(std::function<void(std::function<void()>)> asyncTaskFn);

    void unloadAll();

    auto getDataVector() { return mResources; }

  private:
    foeResourceFns mResourceFns;
    std::shared_mutex mSync;
    std::vector<foeArmature *> mResources;
};

#endif // FOE_RESOURCE_ARMATURE_POOL_HPP