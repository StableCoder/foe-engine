// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/imgui/state.hpp>

#include <foe/external/imgui.h>

void foeImGuiState::setImGuiContext(ImGuiContext *pContext) {
    ImGui::SetCurrentContext(pContext);
    mpImGuiContext = pContext;
}

bool foeImGuiState::addUI(void *pUserData,
                          PFN_foeImGuiRenderMainMenu pMainMenuFn,
                          PFN_foeImGuiRenderCustomUI pCustomFn,
                          char const **ppMenuSets,
                          size_t menuSetCount) {
    if (pUserData == nullptr && pMainMenuFn == nullptr && pCustomFn == nullptr)
        return false;

    std::scoped_lock lock{mSync};

    for (auto const &it : mGuiData) {
        if (pUserData == it.pUserData && pMainMenuFn == it.pMainMenuFn &&
            pCustomFn == it.pCustomFn) {
            return false;
        }
    }

    mGuiData.emplace_back(GuiData{
        .pUserData = pUserData,
        .pMainMenuFn = pMainMenuFn,
        .pCustomFn = pCustomFn,
    });

    for (size_t i = 0; i < menuSetCount; ++i) {
        std::string menuStr{ppMenuSets[i]};

        // If one of the built-in menus, don't need to count them
        if (menuStr == "File") {
            ++mFileMenuCount;
        } else if (menuStr == "Edit") {
            ++mEditMenuCount;
        } else if (menuStr == "View") {
            ++mViewMenuCount;
        } else if (menuStr == "Help") {
            ++mHelpMenuCount;
        } else {
            bool found{false};
            for (auto &it : mMainMenuCounts) {
                if (std::get<0>(it) == menuStr) {
                    ++std::get<1>(it);
                    found = true;
                    break;
                }
            }
            if (!found) {
                mMainMenuCounts.emplace_back(std::make_tuple(menuStr, 1));
            }
        }
    }

    return true;
}

void foeImGuiState::removeUI(void *pUserData,
                             PFN_foeImGuiRenderMainMenu pMainMenuFn,
                             PFN_foeImGuiRenderCustomUI pCustomFn,
                             char const **ppMenuSets,
                             size_t menuSetCount) {
    std::scoped_lock lock{mSync};

    for (auto it = mGuiData.begin(); it != mGuiData.end(); ++it) {
        if (pUserData == it->pUserData && pMainMenuFn == it->pMainMenuFn &&
            pCustomFn == it->pCustomFn) {
            mGuiData.erase(it);

            for (size_t i = 0; i < menuSetCount; ++i) {
                std::string menuStr{ppMenuSets[i]};

                // If one of the built-in menus, don't need to count them
                if (menuStr == "File") {
                    --mFileMenuCount;
                } else if (menuStr == "Edit") {
                    --mEditMenuCount;
                } else if (menuStr == "View") {
                    --mViewMenuCount;
                } else if (menuStr == "Help") {
                    --mHelpMenuCount;
                } else {
                    for (auto &it : mMainMenuCounts) {
                        if (std::get<0>(it) == menuStr) {
                            --std::get<1>(it);
                            break;
                        }
                    }
                }
            }

            break;
        }
    }
}

void foeImGuiState::runUI() {
    std::scoped_lock lock{mSync};

    ImGui::BeginMainMenuBar();

    // File menu
    if (mFileMenuCount > 0 && ImGui::BeginMenu("File")) {
        for (auto const &it : mGuiData) {
            if (it.pMainMenuFn != nullptr) {
                it.pMainMenuFn(mpImGuiContext, it.pUserData, "File");
            }
        }

        ImGui::EndMenu();
    }

    // Edit menu
    if (mEditMenuCount > 0 && ImGui::BeginMenu("Edit")) {
        for (auto const &it : mGuiData) {
            if (it.pMainMenuFn != nullptr) {
                it.pMainMenuFn(mpImGuiContext, it.pUserData, "Edit");
            }
        }

        ImGui::EndMenu();
    }

    // View menu
    if (mViewMenuCount > 0 && ImGui::BeginMenu("View")) {
        for (auto const &it : mGuiData) {
            if (it.pMainMenuFn != nullptr) {
                it.pMainMenuFn(mpImGuiContext, it.pUserData, "View");
            }
        }

        ImGui::EndMenu();
    }

    // Custom menus
    for (auto const &it : mMainMenuCounts) {
        if (std::get<1>(it) > 0) {
            char const *pStr = std::get<0>(it).data();
            if (ImGui::BeginMenu(pStr)) {
                for (auto const &it : mGuiData) {
                    if (it.pMainMenuFn != nullptr) {
                        it.pMainMenuFn(mpImGuiContext, it.pUserData, pStr);
                    }
                }
            }
        }
    }

    // Help menus
    if (mHelpMenuCount > 0 && ImGui::BeginMenu("Help")) {
        for (auto const &it : mGuiData) {
            if (it.pMainMenuFn != nullptr) {
                it.pMainMenuFn(mpImGuiContext, it.pUserData, "Help");
            }
        }

        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();

    // Custom UI
    for (auto const &it : mGuiData) {
        if (it.pCustomFn != nullptr) {
            it.pCustomFn(mpImGuiContext, it.pUserData);
        }
    }
}