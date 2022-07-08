// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/search_paths.hpp>

foeSearchPaths::foeSearchPaths(std::vector<std::filesystem::path> paths) :
    mPaths{std::move(paths)} {}

auto foeSearchPaths::tryGetWriter() noexcept -> Writer {
    if (mSync.try_lock()) {
        return Writer{this};
    }

    return Writer{};
}

auto foeSearchPaths::getWriter() noexcept -> Writer {
    mSync.lock();
    return Writer{this};
}

auto foeSearchPaths::tryGetReader() noexcept -> Reader {
    if (mSync.try_lock_shared()) {
        return Reader{this};
    }

    return Reader{};
}

auto foeSearchPaths::getReader() noexcept -> Reader {
    mSync.lock_shared();
    return Reader{this};
}