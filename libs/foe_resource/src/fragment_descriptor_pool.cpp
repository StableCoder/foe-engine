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

#include <foe/resource/fragment_descriptor_pool.hpp>

#include <foe/resource/fragment_descriptor.hpp>

foeFragmentDescriptorPool::~foeFragmentDescriptorPool() {
    for (auto *pFragmentDescriptor : mFragmentDescriptors) {
        pFragmentDescriptor->decrementRefCount();
    }
}

bool foeFragmentDescriptorPool::add(foeFragmentDescriptor *pFragmentDescriptor) {
    std::scoped_lock lock{mSync};

    for (auto *pOld : mFragmentDescriptors) {
        if (pOld->getName() == pFragmentDescriptor->getName())
            return false;
    }

    pFragmentDescriptor->incrementRefCount();
    mFragmentDescriptors.emplace_back(pFragmentDescriptor);

    return true;
}

foeFragmentDescriptor *foeFragmentDescriptorPool::find(std::string_view name) {
    foeFragmentDescriptor *pFragmentDescriptor{nullptr};

    mSync.lock_shared();
    for (auto *pOld : mFragmentDescriptors) {
        if (pOld->getName() == name) {
            pFragmentDescriptor = pOld;
            break;
        }
    }
    mSync.unlock_shared();

    return pFragmentDescriptor;
}