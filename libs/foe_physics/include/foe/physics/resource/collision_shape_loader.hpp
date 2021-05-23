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
#include <foe/resource/loader_base.hpp>

#include <atomic>
#include <functional>
#include <system_error>

struct foeResourceCreateInfoBase;
struct foePhysCollisionShape;

class FOE_PHYSICS_EXPORT foePhysCollisionShapeLoader : public foeResourceLoaderBase {
  public:
    ~foePhysCollisionShapeLoader();

    std::error_code initialize(std::function<foeResourceCreateInfoBase *(foeId)> importFunction,
                               std::function<void(std::function<void()>)> asynchronousJobs);
    void deinitialize();
    bool initialized() const noexcept;

    void processLoadRequests();
    void processUnloadRequests();

    void requestResourceLoad(foePhysCollisionShape *pCollisionShape);
    void requestResourceUnload(foePhysCollisionShape *pCollisionShape);

  private:
    FOE_PHYSICS_NO_EXPORT void loadResource(foePhysCollisionShape *pCollisionShape);

    FOE_PHYSICS_NO_EXPORT bool mInitialized{false};
    FOE_PHYSICS_NO_EXPORT std::function<foeResourceCreateInfoBase *(foeResourceID)> mImportFunction;

    FOE_PHYSICS_NO_EXPORT std::function<void(std::function<void()>)> mAsyncJobs;
    FOE_PHYSICS_NO_EXPORT std::atomic_int mActiveJobs;
};

#endif // FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_LOADER_HPP