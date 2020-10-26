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

#include <foe/developer_console.hpp>

#include <foe/log.hpp>

FOE_DECLARE_LOG_CATEGORY(ConsoleCommand, Info, Info)
FOE_DEFINE_LOG_CATEGORY(ConsoleCommand)

foeDeveloperConsole *foeDeveloperConsole::instance() {
    static foeDeveloperConsole gDeveloperConsole;
    return &gDeveloperConsole;
}

void foeDeveloperConsole::log(foeLogCategory *pCategory,
                              foeLogLevel level,
                              std::string_view message) {
    std::scoped_lock lock{mSync};

    mEntries.emplace_back(Entry{
        .pCategory = pCategory,
        .level = level,
        .message = std::string{message},
    });

    if (mEntries.size() > mMaxEntries) {
        mEntries.pop_front();
    }
}

void foeDeveloperConsole::exception() {}

bool foeDeveloperConsole::runCommand(std::string_view commandCall) {
    FOE_LOG(ConsoleCommand, Info, commandCall);

    if (!foeCommandStringRunner::runCommand(commandCall)) {
        FOE_LOG(ConsoleCommand, Warning, "Could not find command to call")
        return false;
    }

    return true;
}

size_t foeDeveloperConsole::maxEntries() const noexcept { return mMaxEntries; }

void foeDeveloperConsole::maxEntries(size_t numEntries) noexcept {
    std::scoped_lock lock{mSync};

    mMaxEntries = numEntries;
    while (mEntries.size() > mMaxEntries) {
        mEntries.pop_front();
    }
}

foeDeveloperConsole::foeDeveloperConsole() = default;