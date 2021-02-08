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