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

#ifndef FOE_RESOURCE_MATERIAL_POOL_HPP
#define FOE_RESOURCE_MATERIAL_POOL_HPP

#include <foe/ecs/id.hpp>
#include <foe/resource/export.h>
#include <foe/resource/pool_base.hpp>

#include <shared_mutex>
#include <vector>

struct foeMaterial;

class foeMaterialPool : public foeResourcePoolBase {
  public:
    FOE_RES_EXPORT ~foeMaterialPool();

    FOE_RES_EXPORT bool add(foeMaterial *pMaterial);
    FOE_RES_EXPORT foeMaterial *find(foeId id);

    FOE_RES_EXPORT void unloadAll();

    FOE_RES_EXPORT auto getDataVector() { return mMaterials; }

  private:
    std::shared_mutex mSync;
    std::vector<foeMaterial *> mMaterials;
};

#endif // FOE_RESOURCE_MATERIAL_POOL_HPP