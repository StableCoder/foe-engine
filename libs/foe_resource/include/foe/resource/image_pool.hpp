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

#ifndef FOE_RESOURCE_IMAGE_POOL_HPP
#define FOE_RESOURCE_IMAGE_POOL_HPP

#include <foe/ecs/id.hpp>
#include <foe/resource/export.h>
#include <foe/simulation/core/pool.hpp>

#include <shared_mutex>
#include <vector>

class foeImage;
class foeImageLoader;

class FOE_RES_EXPORT foeImagePool : public foeResourcePoolBase {
  public:
    foeImagePool(foeImageLoader *pImageLoader) : mpImageLoader{pImageLoader} {}
    ~foeImagePool();

    foeImage *add(foeResourceID resource);
    foeImage *findOrAdd(foeResourceID resource);
    foeImage *find(foeId id);

    void unloadAll();

    auto getDataVector() { return mImages; }

  private:
    foeImageLoader *mpImageLoader;
    FOE_RESOURCE_NO_EXPORT std::shared_mutex mSync;
    FOE_RESOURCE_NO_EXPORT std::vector<foeImage *> mImages;
};

#endif // FOE_RESOURCE_IMAGE_POOL_HPP