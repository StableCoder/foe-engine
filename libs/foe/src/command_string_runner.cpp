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

#include <foe/command_string_runner.hpp>

#include <algorithm>
#include <string>

bool foeCommandStringRunner::registerCommand(
    std::string_view commandName, std::function<void(std::string_view)> const &function) {
    std::scoped_lock lock{mSync};
    std::string cmdStr = std::string{commandName};

    // Check to make sure command doesn't already exist
    if (mCommandMap.find(cmdStr) != mCommandMap.end()) {
        return false;
    }

    mCommandMap.insert({cmdStr, function});
    return true;
}

bool foeCommandStringRunner::deregisterCommand(std::string_view commandName) {
    std::scoped_lock lock{mSync};
    auto searchIt = mCommandMap.find(std::string{commandName});
    if (searchIt != mCommandMap.end()) {
        mCommandMap.erase(searchIt);
        return true;
    }

    return false;
}

bool foeCommandStringRunner::runCommand(std::string_view commandCall) {
    auto firstNonSpace = std::string::npos;
    for (size_t i = 0; i < commandCall.size(); ++i) {
        if (std::isspace(commandCall[i]) == 0) {
            firstNonSpace = i;
            break;
        }
    }

    if (firstNonSpace == std::string::npos)
        return false;

    auto firstTokenEnd = std::string::npos;
    for (size_t i = firstNonSpace; i < commandCall.size(); ++i) {
        if (std::isspace(commandCall[i]) != 0) {
            firstTokenEnd = i;
            break;
        }
    }

    std::string cmdSearchStr =
        std::string{commandCall.substr(firstNonSpace, firstTokenEnd - firstNonSpace)};

    std::scoped_lock lock{mSync};
    auto searchIt = mCommandMap.find(cmdSearchStr);
    bool run = searchIt != mCommandMap.end();

    if (run)
        searchIt->second(commandCall);

    return run;
}