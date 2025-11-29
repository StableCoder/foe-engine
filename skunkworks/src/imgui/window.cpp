// Copyright (C) 2021-2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "window.hpp"

#include <fmt/core.h>
#include <foe/external/imgui.h>
#include <foe/imgui/state.hpp>

#include <array>
#include <string_view>

namespace {

std::array<char const *, 1> renderMenus{
    "View",
};

void renderKeyboardUI(KeyboardInput const *pKeyboard) {
    // Handle
    ImGui::Text("Handle: %p", pKeyboard);

    /* These two happen so quickly as to always appear 0 on the UI
    // Unicode Character
    ImGui::Text("Unicode Character: %u", pKeyboard->unicodeChar);

    // Repeat Key
    ImGui::Text("Repeat Key: %u", pKeyboard->repeatKey);
    */

    // Down Keys
    ImGui::Text("Key / Scan Codes Down:");
    ImGui::BeginTable("KeyButtonDownTable", 1, ImGuiTableFlags_BordersOuter);

    for (auto const &it : pKeyboard->downCodes) {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::Text("%u / %u", it.keycode, it.scancode);
    }

    ImGui::EndTable();
}

void renderMouseUI(MouseInput const *pMouse) {
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

bool foeImGuiWindow::addWindow(void *pContext,
                               PFN_WindowTitle pfnTitle,
                               PFN_WindowTerminationCalled pfnTerminationCalled,
                               PFN_WindowSize pfnSize,
                               PFN_WindowVisible pfnVisible,
                               PFN_WindowContentScale pfnContentScale,
                               KeyboardInput const *pKeyboard,
                               MouseInput const *pMouse) {
    for (auto const &it : mWindowList) {
        if (it.pContext == pContext) {
            return false;
        }
    }

    mWindowList.emplace_back(WindowData{
        .pContext = pContext,
        .pfnTitle = pfnTitle,
        .pfnTerminationCalled = pfnTerminationCalled,
        .pfnSize = pfnSize,
        .pfnVisible = pfnVisible,
        .pfnContentScale = pfnContentScale,
        .pKeyboard = pKeyboard,
        .pMouse = pMouse,
        .open = false,
        .focus = false,
    });

    return true;
}

bool foeImGuiWindow::removeWindow(void *pContext) {
    for (auto it = mWindowList.begin(); it != mWindowList.end(); ++it) {
        if (it->pContext == pContext) {
            mWindowList.erase(it);
            return true;
        }
    }

    return false;
}

bool foeImGuiWindow::registerUI(foeImGuiState *pState) {
    return pState->addUI(this, foeImGuiWindow::renderMenuElements, foeImGuiWindow::renderCustomUI,
                         renderMenus.data(), renderMenus.size());
}

void foeImGuiWindow::deregisterUI(foeImGuiState *pState) {
    pState->removeUI(this, foeImGuiWindow::renderMenuElements, foeImGuiWindow::renderCustomUI,
                     renderMenus.data(), renderMenus.size());
}

bool foeImGuiWindow::renderMenuElements(ImGuiContext *pImGuiContext,
                                        void *pUserData,
                                        char const *pMenu) {
    ImGui::SetCurrentContext(pImGuiContext);
    auto *pData = static_cast<foeImGuiWindow *>(pUserData);
    std::string_view menu{pMenu};

    if (menu == "View") {
        return pData->viewMainMenu();
    }

    return false;
}

void foeImGuiWindow::renderCustomUI(ImGuiContext *pImGuiContext, void *pUserData) {
    ImGui::SetCurrentContext(pImGuiContext);
    auto *pData = static_cast<foeImGuiWindow *>(pUserData);

    pData->customUI();
}

bool foeImGuiWindow::viewMainMenu() {
    if (ImGui::BeginMenu("Windows", !mWindowList.empty())) {
        for (auto &it : mWindowList) {
            std::string menuLabel =
                fmt::format("{} : {}", static_cast<void *>(it.pContext), it.pfnTitle(it.pContext));
            if (ImGui::MenuItem(menuLabel.data())) {
                it.open = true;
                it.focus = true;
            }
        }

        ImGui::EndMenu();
    }

    return true;
}

void foeImGuiWindow::customUI() {
    for (auto &it : mWindowList) {
        if (!it.open)
            continue;

        ImGui::SetNextWindowSize(ImVec2(0, 0), 0);
        std::string imguiElementName = fmt::format(
            "Window - ({}) {}", static_cast<void *>(it.pContext), it.pfnTitle(it.pContext));
        if (!ImGui::Begin(imguiElementName.data(), &it.open)) {
            ImGui::End();
            continue;
        }

        // Handle
        ImGui::Text("Handle: %p", static_cast<void *>(it.pContext));

        // Title
        ImGui::Text("Title: %s", it.pfnTitle(it.pContext));

        // Termination called
        if (it.pfnTerminationCalled(it.pContext)) {
            ImGui::Text("Terminate: true");
        } else {
            ImGui::Text("Terminate: false");
        }

        // Size
        int width, height;
        it.pfnSize(it.pContext, &width, &height);
        ImGui::Text("Width/Height: %i x %i", width, height);

        // Visible
        if (it.pfnVisible(it.pContext)) {
            ImGui::Text("Visible: true");
        } else {
            ImGui::Text("Visible: false");
        }

        // Scale
        float xScale, yScale;
        it.pfnContentScale(it.pContext, &xScale, &yScale);
        ImGui::Text("Content Scale: %.2f x %.2f", xScale, yScale);

        // Keyboard
        ImGui::Separator();
        ImGui::Text("Keyboard");
        renderKeyboardUI(it.pKeyboard);

        // Mouse
        ImGui::Separator();
        ImGui::Text("Mouse");
        renderMouseUI(it.pMouse);

        ImGui::End();
    }
}