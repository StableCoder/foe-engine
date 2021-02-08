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

#ifndef FOE_SEARCH_PATHS_HPP
#define FOE_SEARCH_PATHS_HPP

#include <foe/export.h>

#include <filesystem>
#include <shared_mutex>
#include <vector>

class foeSearchPaths {
  public:
    class Writer {
      public:
        FOE_EXPORT Writer() noexcept;
        FOE_EXPORT ~Writer();

        FOE_EXPORT Writer(Writer const &) = delete;
        FOE_EXPORT Writer &operator=(Writer const &) = delete;

        FOE_EXPORT Writer(Writer &&) noexcept;
        FOE_EXPORT Writer &operator=(Writer &&) noexcept;

        FOE_EXPORT bool valid() const noexcept;

        FOE_EXPORT void release() noexcept;

        FOE_EXPORT auto searchPaths() const noexcept -> std::vector<std::filesystem::path> *;

      private:
        friend class foeSearchPaths;

        Writer(foeSearchPaths *pPaths) noexcept;

        foeSearchPaths *mPaths;
    };

    class Reader {
      public:
        FOE_EXPORT Reader() noexcept;
        FOE_EXPORT ~Reader();

        FOE_EXPORT Reader(Reader const &) = delete;
        FOE_EXPORT Reader &operator=(Reader const &) = delete;

        FOE_EXPORT Reader(Reader &&) noexcept;
        FOE_EXPORT Reader &operator=(Reader &&) noexcept;

        FOE_EXPORT bool valid() const noexcept;

        FOE_EXPORT void release() noexcept;

        FOE_EXPORT auto searchPaths() const noexcept -> std::vector<std::filesystem::path> const *;

      private:
        friend class foeSearchPaths;

        Reader(foeSearchPaths *pPaths) noexcept;

        foeSearchPaths *mPaths;
    };

    FOE_EXPORT foeSearchPaths(std::vector<std::filesystem::path> paths = {});

    FOE_EXPORT auto tryGetWriter() noexcept -> Writer;
    FOE_EXPORT auto getWriter() noexcept -> Writer;

    FOE_EXPORT auto tryGetReader() noexcept -> Reader;
    FOE_EXPORT auto getReader() noexcept -> Reader;

  private:
    std::shared_mutex mSync;
    std::vector<std::filesystem::path> mPaths;
};

#endif // FOE_SEARCH_PATHS_HPP