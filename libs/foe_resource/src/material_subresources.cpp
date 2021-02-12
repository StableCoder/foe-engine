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

#include <foe/resource/material.hpp>

#include <foe/resource/fragment_descriptor.hpp>

foeMaterial::SubResources::~SubResources() { reset(); }

foeMaterial::SubResources::SubResources(SubResources &&other) :
    pFragmentDescriptor{std::move(other.pFragmentDescriptor)} {
    other.pFragmentDescriptor = nullptr;
}

auto foeMaterial::SubResources::operator=(SubResources &&other) -> SubResources & {
    reset();

    pFragmentDescriptor = std::move(other.pFragmentDescriptor);
    other.pFragmentDescriptor = nullptr;

    return *this;
}

void foeMaterial::SubResources::reset() {
    if (pFragmentDescriptor != nullptr) {
        pFragmentDescriptor->decrementUseCount();
        pFragmentDescriptor->decrementRefCount();
    }
    pFragmentDescriptor = nullptr;
}