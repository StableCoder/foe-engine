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

#include <foe/simulation/imgui/registrar.hpp>

#include "error_code.hpp"

#include <cstring>

bool foeSimulationImGuiRegistrar::matchFunctionList(ComponentFn componentFn,
                                                    ResourceFn resourceFn,
                                                    LoaderFn loaderFn,
                                                    DisplayFns const *pDisplayFns) const {
    return pDisplayFns->componentFn == componentFn && pDisplayFns->resourceFn == resourceFn &&
           pDisplayFns->loaderFn == loaderFn;
}

auto foeSimulationImGuiRegistrar::registerElements(ComponentFn componentFn,
                                                   ResourceFn resourceFn,
                                                   LoaderFn loaderFn) -> std::error_code {
    if (componentFn == nullptr && resourceFn == nullptr && loaderFn == nullptr)
        return FOE_SIMULATION_IMGUI_ERROR_ALL_PARAMETERS_NULL;

    std::scoped_lock lock{mSync};

    for (auto const &it : mFnLists) {
        if (matchFunctionList(componentFn, resourceFn, loaderFn, &it))
            return FOE_SIMULATION_IMGUI_ERROR_FUNCTIONALITY_ALREADY_REGISTERED;
    }

    mFnLists.emplace_back(DisplayFns{
        .componentFn = componentFn,
        .resourceFn = resourceFn,
        .loaderFn = loaderFn,
    });

    return FOE_SIMULATION_IMGUI_SUCCESS;
}

auto foeSimulationImGuiRegistrar::deregisterElements(ComponentFn componentFn,
                                                     ResourceFn resourceFn,
                                                     LoaderFn loaderFn) -> std::error_code {
    if (componentFn == nullptr && resourceFn == nullptr && loaderFn == nullptr)
        return FOE_SIMULATION_IMGUI_ERROR_ALL_PARAMETERS_NULL;

    std::scoped_lock lock{mSync};

    for (auto it = mFnLists.begin(); it != mFnLists.end(); ++it) {
        if (matchFunctionList(componentFn, resourceFn, loaderFn, &(*it))) {
            mFnLists.erase(it);
            return FOE_SIMULATION_IMGUI_SUCCESS;
        }
    }

    return FOE_SIMULATION_IMGUI_ERROR_FUNCTIONALITY_NOT_REGISTERED;
}

void foeSimulationImGuiRegistrar::displayEntity(foeEntityID entity,
                                                foeComponentPoolBase **ppPools,
                                                size_t poolCount) {
    std::scoped_lock lock{mSync};

    for (auto const &it : mFnLists) {
        if (it.componentFn)
            it.componentFn(entity, ppPools, poolCount);
    }
}

void foeSimulationImGuiRegistrar::displayResource(foeEntityID entity,
                                                  foeResourcePoolBase **ppPools,
                                                  size_t poolCount) {
    std::scoped_lock lock{mSync};

    for (auto const &it : mFnLists) {
        if (it.resourceFn)
            it.resourceFn(entity, ppPools, poolCount);
    }
}