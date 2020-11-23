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

#ifndef FOE_IMGUI_BASE_HPP
#define FOE_IMGUI_BASE_HPP

#include <foe/imgui/export.h>

/**
 * @brief A base class to be inherited for use with the corresponding foeImGuiState class
 *
 * This class is meant to be an otherwise empty base, where the protected functions are called by
 * the friended foeImGuiState class. The functions are called in this order by the foeImGuiState
 * during rendering:
 * - fileMainMenu
 * - viewMainMenu
 * - customMainMenu
 * - customUI
 */
class FOE_IMGUI_EXPORT foeImGuiBase {
  protected:
    /// Call UI elements that belong in the 'File' menu of the main menu
    virtual void fileMainMenu();

    /// Call UI elements that belong in the 'View' menu of the main menu
    virtual void viewMainMenu();

    /// Call UI elements that belong in a custom main menu option
    virtual void customMainMenu();

    /// Any other customized UI calls
    virtual void customUI();

  private:
    friend class foeImGuiState;
};

#endif // FOE_IMGUI_BASE_HPP