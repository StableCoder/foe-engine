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

#include <foe/imgui/state.hpp>

#include <imgui.h>

#include <string>

bool foeImGuiState::addUI(void *pContext,
                          PFN_foeImGuiRenderMainMenu pMainMenuFn,
                          PFN_foeImGuiRenderCustomUI pCustomFn,
                          char const **ppMenuSets,
                          size_t menuSetCount) {
    if (pContext == nullptr && pMainMenuFn == nullptr && pCustomFn == nullptr)
        return false;

    std::scoped_lock lock{mSync};

    for (auto const &it : mGuiData) {
        if (pContext == it.pContext && pMainMenuFn == it.pMainMenuFn && pCustomFn == it.pCustomFn) {
            return false;
        }
    }

    mGuiData.emplace_back(GuiData{
        .pContext = pContext,
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
            bool found;
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

void foeImGuiState::removeUI(void *pContext,
                             PFN_foeImGuiRenderMainMenu pMainMenuFn,
                             PFN_foeImGuiRenderCustomUI pCustomFn,
                             char const **ppMenuSets,
                             size_t menuSetCount) {
    std::scoped_lock lock{mSync};

    for (auto it = mGuiData.begin(); it != mGuiData.end(); ++it) {
        if (pContext == it->pContext && pMainMenuFn == it->pMainMenuFn &&
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
                    bool found;
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
                it.pMainMenuFn(it.pContext, "File");
            }
        }

        ImGui::EndMenu();
    }

    // Edit menu
    if (mEditMenuCount > 0 && ImGui::BeginMenu("Edit")) {
        for (auto const &it : mGuiData) {
            if (it.pMainMenuFn != nullptr) {
                it.pMainMenuFn(it.pContext, "Edit");
            }
        }

        ImGui::EndMenu();
    }

    // View menu
    if (mViewMenuCount > 0 && ImGui::BeginMenu("View")) {
        for (auto const &it : mGuiData) {
            if (it.pMainMenuFn != nullptr) {
                it.pMainMenuFn(it.pContext, "View");
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
                        it.pMainMenuFn(it.pContext, pStr);
                    }
                }
            }
        }
    }

    // Help menus
    if (mHelpMenuCount > 0 && ImGui::BeginMenu("Help")) {
        for (auto const &it : mGuiData) {
            if (it.pMainMenuFn != nullptr) {
                it.pMainMenuFn(it.pContext, "Help");
            }
        }

        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();

    // Custom UI
    for (auto const &it : mGuiData) {
        if (it.pCustomFn != nullptr) {
            it.pCustomFn(it.pContext);
        }
    }
}