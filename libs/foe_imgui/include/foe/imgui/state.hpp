// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_IMGUI_STATE_HPP
#define FOE_IMGUI_STATE_HPP

#include <foe/imgui/export.h>

#include <mutex>
#include <string>
#include <tuple>
#include <vector>

struct ImGuiContext;

// Boolean returns whether anything was rendered (if so, it will add a separator after)
// String is to determine under which menu context is being rendered currently
using PFN_foeImGuiRenderMainMenu = bool (*)(ImGuiContext *, void *, char const *);
using PFN_foeImGuiRenderCustomUI = void (*)(ImGuiContext *, void *);

/// Keeps and renders in order UI items
class foeImGuiState {
  public:
    /** @brief Sets the ImGui context to be passed to UI elements
     * @param pContext Context pointer
     *
     * On Windows, global variables, does not cross DLL library boundaries. This leads to issues
     * where the global ImGui context can be different in different sections of code. To combat
     * this, a single context is meant to be created by the main application, passed to this, which
     * will then proceed to pass it to all rendering calls to ensure a single global context.
     */
    FOE_IMGUI_EXPORT void setImGuiContext(ImGuiContext *pContext);

    /** @brief Adds the rendering of a specific GUI
     * @param pUserData Pointer to the context that is passed in with the given functions
     * @param pMainMenuFn Function to call when rendering main-menu items
     * @param pCustomFn Function to call for rendering custom UI elements
     * @param ppMenuSets Strings that represent the menu dropdowns that the function renders to
     * @param menuSetCount Number of strings in ppMenuSets
     * @return True if added. False if it's all nullptr or already added
     *
     * For the menu sets, this is taken to mean under which menu contexts that the element is
     * rendering UI items to, such as the 'File', 'Edit' or any custom named top-level menu.
     */
    FOE_IMGUI_EXPORT bool addUI(void *pUserData,
                                PFN_foeImGuiRenderMainMenu pMainMenuFn,
                                PFN_foeImGuiRenderCustomUI pCustomFn,
                                char const **ppMenuSets,
                                size_t menuSetCount);

    /** @brief Removes the rendering of specific GUI
     * @param pUserData Pointer to user data that is passed in with the given functions
     * @param pMainMenuFn Function to call when rendering main-menu items
     * @param pCustomFn Function to call for rendering custom UI elements
     * @param ppMenuSets Strings that represent the menu dropdowns that the function renders to
     * @param menuSetCount Number of strings in ppMenuSets
     * @warning There are no checks to ensure the Menu strings are the same as was registered with!
     */
    FOE_IMGUI_EXPORT void removeUI(void *pUserData,
                                   PFN_foeImGuiRenderMainMenu pMainMenuFn,
                                   PFN_foeImGuiRenderCustomUI pCustomFn,
                                   char const **ppMenuSets,
                                   size_t menuSetCount);

    /// Runs all the attached UI items to be rendered
    FOE_IMGUI_EXPORT void runUI();

  private:
    struct GuiData {
        void *pUserData;
        PFN_foeImGuiRenderMainMenu pMainMenuFn;
        PFN_foeImGuiRenderCustomUI pCustomFn;
    };

    /// Global ImGui Context to be passed around
    ImGuiContext *mpImGuiContext{nullptr};
    /// Synchronization primitive
    std::mutex mSync;
    /// Set of UI contexts and functions to be called/rendered
    std::vector<GuiData> mGuiData;

    size_t mFileMenuCount{0};
    size_t mEditMenuCount{0};
    size_t mViewMenuCount{0};
    size_t mHelpMenuCount{0};
    std::vector<std::tuple<std::string, size_t>> mMainMenuCounts;
};

#endif // FOE_IMGUI_STATE_HPP