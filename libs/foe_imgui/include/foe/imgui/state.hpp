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
#include <vector>

class foeImGuiBase;

/// Keeps and renders in order UI items
class foeImGuiState {
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

    /// Runs all the attached UI items to be rendered
    FOE_IMGUI_EXPORT void runUI();

  private:
    /// Synchronization primitive
    std::mutex mSync;
    //// Set of UI elements to be called/rendered
    std::vector<foeImGuiBase *> mUI;
};

#endif // FOE_IMGUI_STATE_HPP