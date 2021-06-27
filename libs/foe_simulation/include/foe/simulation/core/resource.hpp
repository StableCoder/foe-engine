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

#ifndef FOE_SIMULATION_CORE_RESOURCE_HPP
#define FOE_SIMULATION_CORE_RESOURCE_HPP

#include <foe/ecs/id.hpp>
#include <foe/simulation/core/create_info.hpp>

#include <atomic>
#include <memory>
#include <mutex>

struct foeResourceFns;
struct foeResourceCreateInfoBase;

enum class foeResourceState {
    Unloaded,
    Loaded,
    Failed,
};

struct foeResourceBase {
    foeResourceBase(foeResourceID resource, foeResourceFns const *pResourceFns) :
        resource{resource}, pResourceFns{pResourceFns} {}

    foeResourceID getID() const noexcept { return resource; }
    bool getIsLoading() const noexcept { return isLoading; }
    foeResourceState getState() const noexcept { return state; }

    int incrementRefCount() noexcept { return ++refCount; }
    int decrementRefCount() noexcept { return --refCount; }
    int getRefCount() const noexcept { return refCount; }

    int incrementUseCount() noexcept { return ++useCount; }
    int decrementUseCount() noexcept { return --useCount; }
    int getUseCount() const noexcept { return useCount; }

    foeResourceID const resource;
    std::atomic_bool isLoading{false};
    std::atomic<foeResourceState> state{foeResourceState::Unloaded};
    std::atomic_int refCount{0};
    std::atomic_int useCount{0};

    foeResourceFns const *const pResourceFns;

    std::shared_ptr<foeResourceCreateInfoBase> pCreateInfo;

    std::recursive_mutex modifySync;
    std::atomic_uint iteration;
};

#endif // FOE_SIMULATION_CORE_RESOURCE_HPP