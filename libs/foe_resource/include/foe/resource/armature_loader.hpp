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
#include <foe/simulation/core/loader.hpp>

#include <atomic>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

class FOE_RES_EXPORT foeArmatureLoader : public foeResourceLoaderBase {
  public:
    ~foeArmatureLoader();

    std::error_code initialize(
        std::function<foeResourceCreateInfoBase *(foeId)> importFn,
        std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn,
        std::function<void(std::function<void()>)> asynchronousJobs);
    void deinitialize();
    bool initialized() const noexcept;

    void requestResourceLoad(foeArmature *pArmature);
    void requestResourceUnload(foeArmature *pArmature);

  private:
    FOE_RESOURCE_NO_EXPORT void loadResource(foeArmature *pArmature);

    FOE_RESOURCE_NO_EXPORT bool mInitialized{false};
    FOE_RESOURCE_NO_EXPORT std::function<foeResourceCreateInfoBase *(foeId)> mImportFn;
    FOE_RESOURCE_NO_EXPORT std::function<std::filesystem::path(std::filesystem::path)>
        mExternalFileSearchFn;
    FOE_RESOURCE_NO_EXPORT std::function<void(std::function<void()>)> mAsyncJobs;
    FOE_RESOURCE_NO_EXPORT std::atomic_int mActiveJobs;
};

#endif // FOE_RESOURCE_ARMATURE_LOADER_HPP