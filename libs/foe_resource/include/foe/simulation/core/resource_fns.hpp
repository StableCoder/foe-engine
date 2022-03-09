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

#ifndef FOE_SIMULATION_CORE_RESOURCE_FNS_HPP
#define FOE_SIMULATION_CORE_RESOURCE_FNS_HPP

#include <foe/ecs/id.hpp>
#include <foe/resource/resource.h>

#include <functional>
#include <system_error>

struct foeResourceCreateInfoBase;

/**
 * Set of functions common to all foeResource types for importing definitions, loading data and
 * making the importation and loading run asynchronously.
 */
struct foeResourceFns {
    void *pImportContext;
    foeResourceCreateInfoBase *(*pImportFn)(void *, foeResourceID);
    void *pLoadContext;
    void (*pLoadFn)(void *, void *, void (*)(void *, std::error_code));
    void (*pLoadFn2)(void *, foeResource, PFN_foeResourcePostLoad *);
    std::function<void(std::function<void()>)> asyncTaskFn;
};

#endif // FOE_SIMULATION_CORE_RESOURCE_FNS_HPP