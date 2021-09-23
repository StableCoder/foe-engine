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

#ifndef FOE_IMGUI_STATE_HPP
#define FOE_IMGUI_STATE_HPP

#include <foe/imgui/export.h>

#include <mutex>
#include <string>
#include <vector>

class foeImGuiBase;

/// Keeps and renders in order UI items
class foeImGuiState {
    // Boolean returns whether anything was rendered (if so, it will add a separator after)
    // String is to determine under which menu context is being rendered currently
    using PFN_RenderMainMenu = bool (*)(void *, char const *);
    using PFN_RenderCustomUI = void (*)(void *);

  public:
    /** @brief Adds the specific UI element
     * @param pUI Item to attempt to add
     * @return True if added. False if it's nullptr or already added.
     */
    FOE_IMGUI_EXPORT bool addUI(foeImGuiBase *pUI);

    /** @brief Removes the specific UI element
     * @param pUI Item to attempt to remove
     */
    FOE_IMGUI_EXPORT void removeUI(foeImGuiBase *pUI);

    /** @brief Adds the rendering of a specific GUI
     * @param pContext Pointer to the context that is passed in with the given functions
     * @param pMainMenuFn Function to call when rendering main-menu items
     * @param pCustomFn Function to call for rendering custom UI elements
     * @param ppMenuSets Strings that represent the menu dropdowns that the function renders to
     * @param menuSetCount Number of strings in ppMenuSets
     * @return True if added. False if it's all nullptr or already added
     *
     * For the menu sets, this is taken to mean under which menu contexts that the element is
     * rendering UI items to, such as the 'File', 'Edit' or any custom named top-level menu.
     */
    FOE_IMGUI_EXPORT bool addUI(void *pContext,
                                PFN_RenderMainMenu pMainMenuFn,
                                PFN_RenderCustomUI pCustomFn,
                                char const **ppMenuSets,
                                size_t menuSetCount);

    /** @brief Removes the rendering of specific GUI
     * @param pContext Pointer to the context that is passed in with the given functions
     * @param pMainMenuFn Function to call when rendering main-menu items
     * @param pCustomFn Function to call for rendering custom UI elements
     * @param ppMenuSets Strings that represent the menu dropdowns that the function renders to
     * @param menuSetCount Number of strings in ppMenuSets
     */
    FOE_IMGUI_EXPORT void removeUI(void *pContext,
                                   PFN_RenderMainMenu pMainMenuFn,
                                   PFN_RenderCustomUI pCustomFn,
                                   char const **ppMenuSets,
                                   size_t menuSetCount);

    /// Runs all the attached UI items to be rendered
    FOE_IMGUI_EXPORT void runUI();

  private:
    struct GuiData {
        void *pContext;
        PFN_RenderMainMenu pMainMenuFn;
        PFN_RenderCustomUI pCustomFn;
    };

    /// Synchronization primitive
    std::mutex mSync;
    //// Set of UI elements to be called/rendered
    std::vector<foeImGuiBase *> mUI;
    /// Set of UI contexts and functions to be called/rendered
    std::vector<GuiData> mGuiData;

    size_t mFileMenuCount{0};
    size_t mEditMenuCount{0};
    size_t mViewMenuCount{0};
    size_t mHelpMenuCount{0};
    std::vector<std::tuple<std::string, size_t>> mMainMenuCounts;
};

#endif // FOE_IMGUI_STATE_HPP