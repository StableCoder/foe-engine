// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_DEVELOPER_CONSOLE_HPP
#define FOE_DEVELOPER_CONSOLE_HPP

#include <foe/command_string_runner.hpp>
#include <foe/export.h>
#include <foe/log/sink.hpp>

#include <deque>
#include <mutex>

class FOE_EXPORT foeDeveloperConsole : public foeLogSink, public foeCommandStringRunner {
  public:
    foeDeveloperConsole();

    void log(foeLogCategory *pCategory, foeLogLevel level, std::string_view message) final;

    void exception() final;

    bool runCommand(std::string_view commandCall) final;

    size_t maxEntries() const noexcept;
    void maxEntries(size_t numEntries) noexcept;

  protected:
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