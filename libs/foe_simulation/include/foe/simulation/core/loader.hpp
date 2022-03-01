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

#ifndef FOE_SIMULATION_CORE_LOADER_HPP
#define FOE_SIMULATION_CORE_LOADER_HPP

#include <foe/simulation/type_defs.h>

#include <cstddef>
#include <memory>
#include <system_error>

struct foeResourceCreateInfoBase;

struct foeResourceLoaderBase {
    foeResourceLoaderBase(foeSimulationStructureType sType) :
        sType{sType}, pNext{nullptr}, refCount{0}, initCount{0}, gfxInitCount{0} {}
    virtual ~foeResourceLoaderBase() {}

    foeSimulationStructureType sType;
    void *pNext;
    size_t refCount;
    size_t initCount;
    size_t gfxInitCount;
};

#endif // FOE_SIMULATION_CORE_LOADER_HPP