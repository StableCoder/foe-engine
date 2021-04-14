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

#ifndef FOE_RESOURCE_IMAGE_HPP
#define FOE_RESOURCE_IMAGE_HPP

#include <foe/ecs/id.hpp>
#include <foe/graphics/vk/image.hpp>
#include <foe/resource/create_info_base.hpp>
#include <foe/resource/export.h>
#include <foe/resource/load_state.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <string>

class foeImageLoader;
class foeMaterialLoader;

struct foeImageCreateInfo : public foeResourceCreateInfoBase {
    std::string fileName;
};

class foeImage {
  public:
    FOE_RES_EXPORT foeImage(foeId id, foeImageLoader *pLoader);
    FOE_RES_EXPORT ~foeImage();

    FOE_RES_EXPORT foeId getID() const noexcept;
    FOE_RES_EXPORT foeResourceLoadState getLoadState() const noexcept;

    FOE_RES_EXPORT int incrementRefCount() noexcept;
    FOE_RES_EXPORT int decrementRefCount() noexcept;
    FOE_RES_EXPORT int getRefCount() const noexcept;

    FOE_RES_EXPORT int incrementUseCount() noexcept;
    FOE_RES_EXPORT int decrementUseCount() noexcept;
    FOE_RES_EXPORT int getUseCount() const noexcept;

    FOE_RES_EXPORT void requestLoad();
    FOE_RES_EXPORT void requestUnload();

  private:
    friend foeImageLoader;
    friend foeMaterialLoader;

    // General
    foeId id;
    std::atomic<foeResourceLoadState> loadState{foeResourceLoadState::Unloaded};
    std::atomic_int refCount{0};
    std::atomic_int useCount{0};

    // Specialization
    foeImageLoader *const pLoader;

    std::mutex dataWriteLock{};
    struct Data {
        VmaAllocation alloc{};
        VkImage image{};
        VkImageView view{};
        VkSampler sampler{};
    };
    Data data{};
};

#endif // FOE_RESOURCE_IMAGE_HPP