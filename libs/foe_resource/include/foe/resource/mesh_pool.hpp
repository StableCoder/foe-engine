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

#ifndef FOE_RESOURCE_MESH_POOL_HPP
#define FOE_RESOURCE_MESH_POOL_HPP

#include <foe/ecs/resource_id.hpp>
#include <foe/resource/export.h>

#include <shared_mutex>
#include <string_view>
#include <vector>

class foeMesh;

class foeMeshPool {
  public:
    FOE_RES_EXPORT ~foeMeshPool();

    FOE_RES_EXPORT bool add(foeMesh *pMesh);
    FOE_RES_EXPORT foeMesh *find(std::string_view name);
    FOE_RES_EXPORT foeMesh *find(foeResourceID id);

    FOE_RES_EXPORT void unloadAll();

  private:
    std::shared_mutex mSync;
    std::vector<foeMesh *> mMeshs;
};

#endif // FOE_RESOURCE_MESH_POOL_HPP