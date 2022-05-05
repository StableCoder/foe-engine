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

#include <foe/graphics/resource/shader_pool.hpp>

#include <foe/graphics/resource/shader.hpp>
#include <foe/graphics/resource/type_defs.h>

#include <mutex>

foeShaderPool::foeShaderPool(foeResourceFns const &resourceFns) : mResourceFns{resourceFns} {}

foeShaderPool::~foeShaderPool() {
    for (auto const resource : mResources) {
        foeResourceDecrementRefCount(resource);
        foeDestroyResource(resource);
    }
}

foeResource foeShaderPool::add(foeResourceID resource) {
    std::scoped_lock lock{mSync};

    // If it finds it, return nullptr
    for (auto const it : mResources) {
        if (foeResourceGetID(it) == resource) {
            return FOE_NULL_HANDLE;
        }
    }

    // Not found, add it
    foeResource newResource;
    std::error_code errC = foeCreateResource(resource, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER,
                                             &mResourceFns, sizeof(foeShader), &newResource);
    if (errC)
        return FOE_NULL_HANDLE;

    foeResourceIncrementRefCount(newResource);

    mResources.emplace_back(newResource);

    return newResource;
}

foeResource foeShaderPool::findOrAdd(foeResourceID resource) {
    std::scoped_lock lock{mSync};

    for (auto const it : mResources) {
        if (foeResourceGetID(it) == resource) {
            return it;
        }
    }

    // Not found, create it now
    foeResource newResource;
    std::error_code errC = foeCreateResource(resource, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER,
                                             &mResourceFns, sizeof(foeShader), &newResource);
    if (errC)
        return FOE_NULL_HANDLE;

    foeResourceIncrementRefCount(newResource);

    mResources.emplace_back(newResource);

    return newResource;
}

foeResource foeShaderPool::find(foeResourceID resource) {
    foeResource outResource{FOE_NULL_HANDLE};

    mSync.lock_shared();
    for (auto const it : mResources) {
        if (foeResourceGetID(it) == resource) {
            outResource = it;
            break;
        }
    }
    mSync.unlock_shared();

    return outResource;
}

void foeShaderPool::setAsyncTaskFn(PFN_foeScheduleTask scheduleAsyncTask,
                                   void *pScheduleAsyncTaskContext) {
    mResourceFns.scheduleAsyncTask = scheduleAsyncTask;
    mResourceFns.pScheduleAsyncTaskContext = pScheduleAsyncTaskContext;
}

void foeShaderPool::unloadAll() {
    std::scoped_lock lock{mSync};

    for (auto const it : mResources) {
        foeResourceUnload(it, false);
    }
}