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

#ifndef FOE_RESOURCE_FRAGMENT_DESCRIPTOR_HPP
#define FOE_RESOURCE_FRAGMENT_DESCRIPTOR_HPP

#include <foe/graphics/vk/fragment_descriptor.hpp>
#include <foe/resource/export.h>
#include <foe/resource/load_state.hpp>

#include <atomic>
#include <memory>
#include <mutex>

class foeFragmentDescriptorLoader;

struct foeFragmentDescriptorSourceBase {
    virtual ~foeFragmentDescriptorSourceBase() = default;
};

class foeFragmentDescriptor {
  public:
    FOE_RES_EXPORT foeFragmentDescriptor(foeFragmentDescriptorLoader *pLoader);
    FOE_RES_EXPORT ~foeFragmentDescriptor();

    FOE_RES_EXPORT foeResourceLoadState getLoadState() const noexcept;

    FOE_RES_EXPORT int incrementRefCount() noexcept;
    FOE_RES_EXPORT int decrementRefCount() noexcept;
    FOE_RES_EXPORT int getRefCount() const noexcept;

    FOE_RES_EXPORT int incrementUseCount() noexcept;
    FOE_RES_EXPORT int decrementUseCount() noexcept;
    FOE_RES_EXPORT int getUseCount() const noexcept;

    FOE_RES_EXPORT foeGfxVkFragmentDescriptor *getFragmentDescriptor() const noexcept;

    FOE_RES_EXPORT void setSourceExternalFile(std::string_view file);

  private:
    friend foeFragmentDescriptorLoader;

    void requestResourceLoad();

    // General
    std::atomic<foeResourceLoadState> loadState{foeResourceLoadState::Unloaded};
    std::atomic_int refCount{0};
    std::atomic_int useCount{0};

    // Specialization
    foeFragmentDescriptorLoader *pLoader;
    std::shared_ptr<foeFragmentDescriptorSourceBase> pSourceData{nullptr};

    std::mutex dataWriteLock{};
    struct Data {
        foeFragmentDescriptorSourceBase *pLoadedSource;
        foeGfxVkFragmentDescriptor *pGfxFragDescriptor;
    };
    Data data{};
};

#endif // FOE_RESOURCE_FRAGMENT_DESCRIPTOR_HPP