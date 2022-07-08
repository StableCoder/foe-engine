// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/wsi/imgui/window.hpp>

#include <fmt/core.h>
#include <foe/imgui/state.hpp>
#include <foe/wsi/keyboard.hpp>
#include <foe/wsi/mouse.hpp>
#include <imgui.h>

#include <array>
#include <string_view>

namespace {

std::array<char const *, 1> renderMenus{
    "View",
};

void renderKeyboardUI(foeWsiKeyboard const *pKeyboard) {
    // Handle
    ImGui::Text("Handle: %p", pKeyboard);

    /* These two happen so quickly as to always appear 0 on the UI
    // Unicode Character
    ImGui::Text("Unicode Character: %u", pKeyboard->unicodeChar);

    // Repeat Key
    ImGui::Text("Repeat Key: %u", pKeyboard->repeatKey);
    */

    // Down Keys
    ImGui::Text("Keys Down:");
    ImGui::BeginTable("KeyButtonDownTable", 1, ImGuiTableFlags_BordersOuter);

    for (auto const &it : pKeyboard->downKeys) {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::Text("%u", it);
    }

    ImGui::EndTable();
}

void renderMouseUI(foeWsiMouse const *pMouse) {
    // Handle
    ImGui::Text("Handle: %p", pMouse);

    // Within Window Bounds
    if (pMouse->inWindow) {
        ImGui::Text("In Window: true");
    } else {
        ImGui::Text("In Window: false");
    }

    // Position
    ImGui::Text("Position: X %.2f Y %.2f", pMouse->position.x, pMouse->position.y);

    // Scroll
    ImGui::Text("Scroll: X %.2f Y %.2f", pMouse->scroll.x, pMouse->scroll.y);

    // Buttons Down
    ImGui::Text("Buttons Down:");
    ImGui::BeginTable("MouseButtonDownTable", 1, ImGuiTableFlags_BordersOuter);

    for (auto const &it : pMouse->downButtons) {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::Text("%u", it);
    }

    ImGui::EndTable();
}

} // namespace

bool foeWsiImGuiWindow::addWindow(foeWsiWindow window) {
    for (auto const &it : mWindowList) {
        if (it.window == window) {
            return false;
        }
    }

    mWindowList.emplace_back(WindowData{
        .window = window,
        .open = false,
        .focus = false,
    });

    return true;
}

bool foeWsiImGuiWindow::removeWindow(foeWsiWindow window) {
    for (auto it = mWindowList.begin(); it != mWindowList.end(); ++it) {
        if (it->window == window) {
            mWindowList.erase(it);
            return true;
        }
    }

    return false;
}

bool foeWsiImGuiWindow::registerUI(foeImGuiState *pState) {
    return pState->addUI(this, foeWsiImGuiWindow::renderMenuElements,
                         foeWsiImGuiWindow::renderCustomUI, renderMenus.data(), renderMenus.size());
}

void foeWsiImGuiWindow::deregisterUI(foeImGuiState *pState) {
    pState->removeUI(this, foeWsiImGuiWindow::renderMenuElements, foeWsiImGuiWindow::renderCustomUI,
                     renderMenus.data(), renderMenus.size());
}

bool foeWsiImGuiWindow::renderMenuElements(ImGuiContext *pImGuiContext,
                                           void *pUserData,
                                           char const *pMenu) {
    ImGui::SetCurrentContext(pImGuiContext);
    auto *pData = static_cast<foeWsiImGuiWindow *>(pUserData);
    std::string_view menu{pMenu};

    if (menu == "View") {
        return pData->viewMainMenu();
    }

    return false;
}

void foeWsiImGuiWindow::renderCustomUI(ImGuiContext *pImGuiContext, void *pUserData) {
    ImGui::SetCurrentContext(pImGuiContext);
    auto *pData = static_cast<foeWsiImGuiWindow *>(pUserData);

    pData->customUI();
}

bool foeWsiImGuiWindow::viewMainMenu() {
    if (ImGui::BeginMenu("Windows", !mWindowList.empty())) {
        for (auto &it : mWindowList) {
            std::string menuLabel = fmt::format("{} : {}", static_cast<void *>(it.window),
                                                foeWsiWindowGetTitle(it.window));
            if (ImGui::MenuItem(menuLabel.data())) {
                it.open = true;
                it.focus = true;
            }
        }

        ImGui::EndMenu();
    }

    return true;
}

void foeWsiImGuiWindow::customUI() {
    for (auto &it : mWindowList) {
        if (!it.open)
            continue;

        ImGui::SetNextWindowSize(ImVec2(0, 0), 0);
        std::string imguiElementName =
            fmt::format("foeWsiWindow - ({}) {}", static_cast<void *>(it.window),
                        foeWsiWindowGetTitle(it.window));
        if (!ImGui::Begin(imguiElementName.data(), &it.open)) {
            ImGui::End();
            continue;
        }

        // Handle
        ImGui::Text("Handle: %p", static_cast<void *>(it.window));

        // Title
        ImGui::Text("Title: %s", foeWsiWindowGetTitle(it.window));

        // Termination called
        if (foeWsiWindowGetShouldClose(it.window)) {
            ImGui::Text("Terminate: true");
        } else {
            ImGui::Text("Terminate: false");
        }

        // Size
        int width, height;
        foeWsiWindowGetSize(it.window, &width, &height);
        ImGui::Text("Width/Height: %i x %i", width, height);

        // Visible
        if (foeWsiWindowVisible(it.window)) {
            ImGui::Text("Visible: true");
        } else {
            ImGui::Text("Visible: false");
        }

        // Scale
        float xScale, yScale;
        foeWsiWindowGetContentScale(it.window, &xScale, &yScale);
        ImGui::Text("Content Scale: %.2f x %.2f", xScale, yScale);

        // Keyboard
        ImGui::Separator();
        ImGui::Text("Keyboard");
        renderKeyboardUI(foeWsiGetKeyboard(it.window));

        // Mouse
        ImGui::Separator();
        ImGui::Text("Mouse");
        renderMouseUI(foeWsiGetMouse(it.window));

        ImGui::End();
    }
}