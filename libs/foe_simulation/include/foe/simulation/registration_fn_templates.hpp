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

#ifndef FOE_SIMULATION_REGISTRATION_FN_TEMPLATES_HPP
#define FOE_SIMULATION_REGISTRATION_FN_TEMPLATES_HPP

/// Searches for the given type, dynamically casting the given iterators
template <typename SearchType, typename InputIt>
SearchType *search(InputIt start, InputIt end) noexcept {
    for (; start != end; ++start) {
        auto *dynPtr = dynamic_cast<SearchType *>(*start);
        if (dynPtr)
            return dynPtr;
    }

    return nullptr;
}

template <typename SearchType, typename InputIt>
SearchType *searchLoaders(InputIt start, InputIt end) noexcept {
    for (; start != end; ++start) {
        auto *dynPtr = dynamic_cast<SearchType *>(start->pLoader);
        if (dynPtr)
            return dynPtr;
    }

    return nullptr;
}

template <typename DestroyType, typename InType>
void searchAndDestroy(InType &ptr) noexcept {
    auto *dynPtr = dynamic_cast<DestroyType *>(ptr);
    if (dynPtr && (--dynPtr->refCount == 0)) {
        delete dynPtr;
        ptr = nullptr;
    }
}

template <typename DestroyType, typename InType>
void searchAndDeinit(InType &ptr) noexcept {
    auto *dynPtr = dynamic_cast<DestroyType *>(ptr);
    if (dynPtr && (--dynPtr->initCount == 0)) {
        dynPtr->deinitialize();
    }
}

template <typename DestroyType, typename InType>
void searchAndDeinitGraphics(InType &ptr) noexcept {
    auto *dynPtr = dynamic_cast<DestroyType *>(ptr);
    if (dynPtr && (--dynPtr->initCount == 0)) {
        dynPtr->deinitializeGraphics();
    }
}

#endif // FOE_SIMULATION_REGISTRATION_FN_TEMPLATES_HPP