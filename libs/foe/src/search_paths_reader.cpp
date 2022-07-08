// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

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