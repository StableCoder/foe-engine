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
#include <foe/simulation/core/resource.hpp>

struct foeArmature : public foeResourceBase {
    foeArmature(foeResourceID resource, foeResourceFns const *pResourceFns);
    ~foeArmature();

    void loadCreateInfo();
    void loadResource(bool refreshCreateInfo);
    void unloadResource();

    struct Data {
        void *pUnloadContext{nullptr};
        void (*pUnloadFn)(void *, void *, uint32_t, bool){nullptr};
        std::shared_ptr<foeResourceCreateInfoBase> pCreateInfo;

        std::vector<foeArmatureNode> armature;
        std::vector<foeAnimation> animations;
    } data;
};

#endif // FOE_RESOURCE_ARMATURE_HPP