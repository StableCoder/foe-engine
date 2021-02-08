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

#include <foe/search_paths.hpp>

foeSearchPaths::Reader::Reader() noexcept : mPaths{nullptr} {}

foeSearchPaths::Reader::~Reader() { release(); }

foeSearchPaths::Reader::Reader(Reader &&other) noexcept : mPaths{other.mPaths} {
    other.mPaths = nullptr;
}

auto foeSearchPaths::Reader::operator=(Reader &&other) noexcept -> Reader & {
    if (mPaths != nullptr) {
        mPaths->mSync.unlock_shared();
    }

    mPaths = other.mPaths;
    other.mPaths = nullptr;

    return *this;
}

bool foeSearchPaths::Reader::valid() const noexcept { return mPaths != nullptr; }

void foeSearchPaths::Reader::release() noexcept {
    if (mPaths != nullptr) {
        mPaths->mSync.unlock_shared();
        mPaths = nullptr;
    }
}

auto foeSearchPaths::Reader::searchPaths() const noexcept
    -> std::vector<std::filesystem::path> const * {
    if (mPaths != nullptr) {
        return &mPaths->mPaths;
    }

    return nullptr;
}

foeSearchPaths::Reader::Reader(foeSearchPaths *pPaths) noexcept : mPaths{pPaths} {}