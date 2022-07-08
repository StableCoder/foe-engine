// Copyright (C) 2020-2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/command_string_runner.hpp>

#include <algorithm>
#include <string>

foeCommandStringRunner::foeCommandStringRunner() = default;

foeCommandStringRunner::~foeCommandStringRunner() = default;

bool foeCommandStringRunner::registerCommand(std::string_view commandName, CommandFn &&commandFn) {
    std::scoped_lock lock{mSync};
    std::string cmdStr = std::string{commandName};

    // Check to make sure command doesn't already exist
    if (mCommandMap.find(cmdStr) != mCommandMap.end()) {
        return false;
    }

    mCommandMap.insert({cmdStr, commandFn});
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