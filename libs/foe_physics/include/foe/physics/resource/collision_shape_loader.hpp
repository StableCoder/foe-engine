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

#ifndef FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_LOADER_HPP
#define FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_LOADER_HPP

#include <foe/ecs/id.hpp>
#include <foe/physics/export.h>

#include <atomic>
#include <functional>
#include <system_error>

struct foeResourceCreateInfoBase;
struct foePhysCollisionShape;

class foePhysCollisionShapeLoader {
  public:
    FOE_PHYSICS_EXPORT ~foePhysCollisionShapeLoader();

    FOE_PHYSICS_EXPORT std::error_code initialize(
        std::function<foeResourceCreateInfoBase *(foeId)> importFunction,
        std::function<void(std::function<void()>)> asynchronousJobs);
    FOE_PHYSICS_EXPORT void deinitialize();
    FOE_PHYSICS_EXPORT bool initialized() const noexcept;

    FOE_PHYSICS_EXPORT void processLoadRequests();
    FOE_PHYSICS_EXPORT void processUnloadRequests();

    FOE_PHYSICS_EXPORT void requestResourceLoad(foePhysCollisionShape *pCollisionShape);
    FOE_PHYSICS_EXPORT void requestResourceUnload(foePhysCollisionShape *pCollisionShape);

  private:
    void loadResource(foePhysCollisionShape *pCollisionShape);

    bool mInitialized{false};
    std::function<foeResourceCreateInfoBase *(foeResourceID)> mImportFunction;

    std::function<void(std::function<void()>)> mAsyncJobs;
    std::atomic_int mActiveJobs;
};

#endif // FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_LOADER_HPP