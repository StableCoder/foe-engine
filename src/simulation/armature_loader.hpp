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

#ifndef FOE_RESOURCE_ARMATURE_LOADER_HPP
#define FOE_RESOURCE_ARMATURE_LOADER_HPP

#include "armature.hpp"
#include <foe/resource/resource.h>
#include <foe/simulation/core/create_info.hpp>

#include <filesystem>
#include <functional>
#include <mutex>
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

class foeArmatureLoader {
  public:
    std::error_code initialize(
        std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn);
    void deinitialize();
    bool initialized() const noexcept;

    void maintenance();

    static bool canProcessCreateInfo(foeResourceCreateInfoBase *pCreateInfo);
    static void load(void *pLoader,
                     foeResource resource,
                     std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
                     PFN_foeResourcePostLoad *pPostLoadFn);

  private:
    static void unloadResource(void *pContext,
                               foeResource resource,
                               uint32_t resourceIteration,
                               PFN_foeResourceUnloadCall *pUnloadCallFn,
                               bool immediateUnload);

    void load(foeResource resource,
              std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
              PFN_foeResourcePostLoad *pPostLoadFn);

    std::function<std::filesystem::path(std::filesystem::path)> mExternalFileSearchFn;

    struct LoadData {
        foeResource resource;
        std::shared_ptr<foeResourceCreateInfoBase> pCreateInfo;
        PFN_foeResourcePostLoad *pPostLoadFn;
        foeArmature data;
    };

    std::mutex mLoadSync;
    std::vector<LoadData> mToLoad;

    struct UnloadData {
        foeResource resource;
        uint32_t iteration;
        PFN_foeResourceUnloadCall *pUnloadCallFn;
    };

    std::mutex mUnloadRequestsSync;
    std::vector<UnloadData> mUnloadRequests;
};

#endif // FOE_RESOURCE_ARMATURE_LOADER_HPP