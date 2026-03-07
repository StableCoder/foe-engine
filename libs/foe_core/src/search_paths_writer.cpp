// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/search_paths.hpp>

foeSearchPaths::Writer::Writer() noexcept : mPaths{nullptr} {}

foeSearchPaths::Writer::~Writer() { release(); }

foeSearchPaths::Writer::Writer(Writer &&other) noexcept : mPaths{other.mPaths} {
    other.mPaths = nullptr;
}

auto foeSearchPaths::Writer::operator=(Writer &&other) noexcept -> Writer & {
    if (mPaths != nullptr) {
        mPaths->mSync.unlock();
    }

    mPaths = other.mPaths;
    other.mPaths = nullptr;

    return *this;
}

bool foeSearchPaths::Writer::valid() const noexcept { return mPaths != nullptr; }

void foeSearchPaths::Writer::release() noexcept {
    if (mPaths != nullptr) {
        mPaths->mSync.unlock();
        mPaths = nullptr;
    }
}

auto foeSearchPaths::Writer::searchPaths() const noexcept -> std::vector<std::filesystem::path> * {
    if (mPaths != nullptr) {
        return &mPaths->mPaths;
    }

    return nullptr;
}

foeSearchPaths::Writer::Writer(foeSearchPaths *pPaths) noexcept : mPaths{pPaths} {}