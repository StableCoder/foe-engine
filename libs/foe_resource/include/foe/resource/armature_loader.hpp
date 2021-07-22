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
#include <foe/simulation/core/create_info.hpp>
#include <foe/simulation/core/loader.hpp>

#include <filesystem>
#include <functional>
#include <string>
#include <system_error>
#include <vector>

struct AnimationImportInfo {
    std::string file;
    std::vector<std::string> animationNames;
};

struct foeArmatureCreateInfo : public foeResourceCreateInfoBase {
    std::string fileName;
    std::string rootArmatureNode;
    std::vector<AnimationImportInfo> animations;
};

class FOE_RES_EXPORT foeArmatureLoader : public foeResourceLoaderBase {
  public:
    std::error_code initialize(
        std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn);
    void deinitialize();
    bool initialized() const noexcept;

    void maintenance();

    bool canProcessCreateInfo(foeResourceCreateInfoBase *pCreateInfo) final;
    void load(void *pResource,
              std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
              void (*pPostLoadFn)(void *, std::error_code)) final;

  private:
    static void unloadResource(void *pContext,
                               void *pResource,
                               uint32_t resourceIteration,
                               bool immediateUnload);

    std::function<std::filesystem::path(std::filesystem::path)> mExternalFileSearchFn;

    struct LoadData {
        foeArmature *pArmature;
        void (*pPostLoadFn)(void *, std::error_code);
        foeArmature::Data data;
    };

    std::mutex mLoadSync;
    std::vector<LoadData> mToLoad;

    struct UnloadData {
        foeArmature *pArmature;
        uint32_t iteration;
    };

    std::mutex mUnloadRequestsSync;
    std::vector<UnloadData> mUnloadRequests;
};

#endif // FOE_RESOURCE_ARMATURE_LOADER_HPP