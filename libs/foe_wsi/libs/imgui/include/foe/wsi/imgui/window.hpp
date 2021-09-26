/*
    Copyright (C) 2021 George Cave.

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

#ifndef FOE_WSI_IMGUI_WINDOW_HPP
#define FOE_WSI_IMGUI_WINDOW_HPP

#include <foe/wsi/imgui/export.h>
#include <foe/wsi/window.h>

#include <vector>

class foeImGuiState;

class foeWsiImGuiWindow {
  public:
    FOE_WSI_IMGUI_EXPORT bool addWindow(foeWsiWindow window);
    FOE_WSI_IMGUI_EXPORT bool removeWindow(foeWsiWindow window);

    FOE_WSI_IMGUI_EXPORT bool registerUI(foeImGuiState *pState);
    FOE_WSI_IMGUI_EXPORT void deregisterUI(foeImGuiState *pState);

  private:
    struct WindowData {
        foeWsiWindow window;
        bool open;
        bool focus;
    };

    static bool renderMenuElements(void *pContext, char const *pMenu);
    static void renderCustomUI(void *pContext);

    bool viewMainMenu();
    void customUI();

    std::vector<WindowData> mWindowList;
};

#endif // FOE_WSI_IMGUI_WINDOW_HPP