// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IMGUI_DEVELOPER_CONSOLE_HPP
#define IMGUI_DEVELOPER_CONSOLE_HPP

#include <foe/log/sink.hpp>

#include <mutex>
#include <queue>
#include <string>

class foeImGuiState;
struct ImGuiContext;

class foeImGuiDeveloperConsole : public foeLogSink {
  public:
    bool registerUI(foeImGuiState *pState);
    void deregisterUI(foeImGuiState *pState);

    void log(char const *pCategoryName, foeLogLevel level, std::string_view message) final;
    void exception() final;

    size_t maxEntries() const noexcept;
    void maxEntries(size_t numEntries) noexcept;

  private:
    static bool renderMenuElements(ImGuiContext *pImGuiContext, void *pUserData, char const *pMenu);
    static void renderCustomUI(ImGuiContext *pImGuiContext, void *pUserData);

    bool viewMainMenu();
    void customUI();

    // UI
    std::string mBuffer;
    bool mOpen{false};
    bool mFocus{false};

    // Log Sink
    struct Entry {
        std::string category;
        foeLogLevel level;
        std::string message;
    };

    size_t mMaxEntries = 250;
    std::mutex mSync;
    std::deque<Entry> mEntries;
};

#endif // IMGUI_DEVELOPER_CONSOLE_HPP