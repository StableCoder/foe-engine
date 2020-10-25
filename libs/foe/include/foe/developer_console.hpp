/*
    Copyright (C) 2020 George Cave.

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

#ifndef FOE_DEVELOPER_CONSOLE_HPP
#define FOE_DEVELOPER_CONSOLE_HPP

#include <foe/command_string_runner.hpp>
#include <foe/export.h>
#include <foe/log/sink.hpp>

#include <deque>
#include <mutex>

class FOE_EXPORT foeDeveloperConsole : public foeLogSink, public foeCommandStringRunner {
  public:
    static foeDeveloperConsole *instance();

    void log(foeLogCategory *pCategory, foeLogLevel level, std::string_view message) final;

    void exception() final;

    bool runCommand(std::string_view commandCall) final;

    size_t maxEntries() const noexcept;
    void maxEntries(size_t numEntries) noexcept;

  private:
    foeDeveloperConsole() = default;

    struct Entry {
        foeLogCategory *pCategory;
        foeLogLevel level;
        std::string message;
    };

    size_t mMaxEntries = 250;
    std::mutex mSync;
    std::deque<Entry> mEntries;
};

#endif // FOE_DEVELOPER_CONSOLE_HPP