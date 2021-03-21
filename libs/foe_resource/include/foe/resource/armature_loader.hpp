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

#ifndef FOE_RESOURCE_ARMATURE_LOADER_HPP
#define FOE_RESOURCE_ARMATURE_LOADER_HPP

#include <foe/resource/armature.hpp>
#include <foe/resource/export.h>

#include <atomic>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

class foeArmatureLoader {
  public:
    FOE_RES_EXPORT ~foeArmatureLoader();

    FOE_RES_EXPORT std::error_code initialize(
        std::function<bool(
            std::string_view, std::string &, std::string &, std::vector<AnimationImportInfo> &)>
            importFunction,
        std::function<void(std::function<void()>)> asynchronousJobs);
    FOE_RES_EXPORT void deinitialize();
    FOE_RES_EXPORT bool initialized() const noexcept;

    FOE_RES_EXPORT void requestResourceLoad(foeArmature *pArmature);
    FOE_RES_EXPORT void requestResourceUnload(foeArmature *pArmature);

  private:
    void loadResource(foeArmature *pArmature);

    bool mInitialized{false};
    std::function<bool(
        std::string_view, std::string &, std::string &, std::vector<AnimationImportInfo> &)>
        mImportFunction;
    std::function<void(std::function<void()>)> mAsyncJobs;
    std::atomic_int mActiveJobs;
};

#endif // FOE_RESOURCE_ARMATURE_LOADER_HPP