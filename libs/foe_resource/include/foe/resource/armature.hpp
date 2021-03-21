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

#ifndef FOE_RESOURCE_ARMATURE_HPP
#define FOE_RESOURCE_ARMATURE_HPP

#include <foe/model/animation.hpp>
#include <foe/model/armature.hpp>
#include <foe/resource/export.h>
#include <foe/resource/load_state.hpp>

#include <atomic>
#include <mutex>
#include <string>
#include <string_view>

class foeArmatureLoader;

struct AnimationImportInfo {
    std::string file;
    std::string animationName;
};

struct foeArmature {
  public:
    FOE_RES_EXPORT foeArmature(std::string_view name, foeArmatureLoader *pLoader);
    FOE_RES_EXPORT ~foeArmature();

    FOE_RES_EXPORT std::string_view getName() const noexcept;
    FOE_RES_EXPORT foeResourceLoadState getLoadState() const noexcept;

    FOE_RES_EXPORT int incrementRefCount() noexcept;
    FOE_RES_EXPORT int decrementRefCount() noexcept;
    FOE_RES_EXPORT int getRefCount() const noexcept;

    FOE_RES_EXPORT int incrementUseCount() noexcept;
    FOE_RES_EXPORT int decrementUseCount() noexcept;
    FOE_RES_EXPORT int getUseCount() const noexcept;

    FOE_RES_EXPORT void requestLoad();
    FOE_RES_EXPORT void requestUnload();

  private:
    friend foeArmatureLoader;

    // General
    std::string const name;
    std::atomic<foeResourceLoadState> loadState{foeResourceLoadState::Unloaded};
    std::atomic_int refCount{0};
    std::atomic_int useCount{0};

    // Specialization
    foeArmatureLoader *const pLoader;

    std::mutex dataWriteLock{};

  public:
    struct Data {
        std::vector<foeArmatureNode> armature;
        std::vector<foeAnimation> animations;
    } data;
};

#endif // FOE_RESOURCE_ARMATURE_HPP