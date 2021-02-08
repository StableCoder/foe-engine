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

/**
 * @brief Encapsulates a list of filesystem paths that are thread-safe
 *
 * Read/write locks can be acquired and the path list read/modified through the available accessors.
 */
class foeSearchPaths {
  public:
    /// Encapsulates the ability to safely read/modify a SearchPaths object that spawned it.
    class Writer {
      public:
        FOE_EXPORT Writer() noexcept;
        FOE_EXPORT ~Writer();

        FOE_EXPORT Writer(Writer const &) = delete;
        FOE_EXPORT Writer &operator=(Writer const &) = delete;

        FOE_EXPORT Writer(Writer &&) noexcept;
        FOE_EXPORT Writer &operator=(Writer &&) noexcept;

        /// Returns true if this object is valid and can access a foeSearchPaths object.
        FOE_EXPORT bool valid() const noexcept;

        /// If a lock is held, releases it, invalidating this accessor
        FOE_EXPORT void release() noexcept;

        /// Returns a modifiable pointer to a vector of search paths
        FOE_EXPORT auto searchPaths() const noexcept -> std::vector<std::filesystem::path> *;

      private:
        friend class foeSearchPaths;

        /**
         * @brief Private constructor
         * @param pCreator Pointer to the creating object
         *
         * Private so that only the foeSearchPaths class can create this, and only when it
         * successfully acquires a lock
         */
        Writer(foeSearchPaths *pPaths) noexcept;

        /// Pointer to the object that this accessor can be used to access
        foeSearchPaths *mPaths;
    };

    /// Encapsulates the ability to safely read the foeSearchPaths object that spawned it.
    class Reader {
      public:
        FOE_EXPORT Reader() noexcept;
        FOE_EXPORT ~Reader();

        FOE_EXPORT Reader(Reader const &) = delete;
        FOE_EXPORT Reader &operator=(Reader const &) = delete;

        FOE_EXPORT Reader(Reader &&) noexcept;
        FOE_EXPORT Reader &operator=(Reader &&) noexcept;

        /// Returns true if this object is valid and can access a foeSearchPaths object.
        FOE_EXPORT bool valid() const noexcept;

        /// If a lock is held, releases it, invalidating this accessor
        FOE_EXPORT void release() noexcept;

        /// Returns a const pointer to a list of search paths
        FOE_EXPORT auto searchPaths() const noexcept -> std::vector<std::filesystem::path> const *;

      private:
        friend class foeSearchPaths;

        /** Private constructor
         * @param pCreator Pointer to the creating object
         *
         * Private so that only the SearchPaths class can create this, and only when it successfully
         * acquires a lock
         */
        Reader(foeSearchPaths *pPaths) noexcept;

        /// Pointer to the object that this accessor can be used to access
        foeSearchPaths *mPaths;
    };

    FOE_EXPORT foeSearchPaths(std::vector<std::filesystem::path> paths = {});

    /** Attempts to acquire a writing lock
     * @return A valid WriteAccessor if the lock was acquired, an invalid one otherwise
     * @note Non-blocking
     */
    FOE_EXPORT auto tryGetWriter() noexcept -> Writer;

    /** Acquires a writing lock, and blocks until it is acquired
     * @return A valid WriteAccessor
     * @note Blocking
     */
    FOE_EXPORT auto getWriter() noexcept -> Writer;

    /** Attempts to acquire a reading lock
     * @return A valid ReadAccessor if the lock was acquired, an invalid one otherwise
     * @note Non-blocking
     */
    FOE_EXPORT auto tryGetReader() noexcept -> Reader;

    /** Acquires a reading lock, and blocks until it is acquired
     * @return A valid ReadAccessor
     * @note Blocking
     */
    FOE_EXPORT auto getReader() noexcept -> Reader;

  private:
    /// Synchronization primitive that regulates access
    std::shared_mutex mSync;
    /// The list of actual search paths
    std::vector<std::filesystem::path> mPaths;
};

#endif // FOE_SEARCH_PATHS_HPP