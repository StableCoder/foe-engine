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

#ifndef FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_LOADER_HPP
#define FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_LOADER_HPP

#include <foe/ecs/id.hpp>
#include <foe/physics/export.h>
#include <foe/physics/resource/collision_shape.hpp>
#include <foe/simulation/core/create_info.hpp>
#include <foe/simulation/core/loader.hpp>
#include <glm/glm.hpp>

#include <mutex>
#include <system_error>
#include <vector>

struct foeCollisionShapeCreateInfo : public foeResourceCreateInfoBase {
    glm::vec3 boxSize;
};

class FOE_PHYSICS_EXPORT foeCollisionShapeLoader : public foeResourceLoaderBase {
  public:
    foeCollisionShapeLoader();
    ~foeCollisionShapeLoader();

    std::error_code initialize();
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

    struct LoadData {
        foeCollisionShape *pCollisionShape;
        void (*pPostLoadFn)(void *, std::error_code);
        foeCollisionShape::Data data;
    };

    std::mutex mLoadSync;
    std::vector<LoadData> mToLoad;

    struct UnloadData {
        foeCollisionShape *pCollisionShape;
        uint32_t iteration;
    };

    std::mutex mUnloadRequestsSync;
    std::vector<UnloadData> mUnloadRequests;
};

#endif // FOE_PHYSICS_RESOURCE_COLLISION_SHAPE_LOADER_HPP