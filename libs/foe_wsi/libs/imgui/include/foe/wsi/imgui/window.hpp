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

#include <foe/imgui/base.hpp>
#include <foe/wsi/imgui/export.h>
#include <foe/wsi/window.h>

#include <vector>

class FOE_WSI_IMGUI_EXPORT foeWsiImGuiWindow : public foeImGuiBase {
  public:
    bool addWindow(foeWsiWindow window);
    bool removeWindow(foeWsiWindow window);

  private:
    struct WindowData {
        foeWsiWindow window;
        bool open;
        bool focus;
    };

    void viewMainMenu();
    void customUI();

    std::vector<WindowData> mWindowList;
};

#endif // FOE_WSI_IMGUI_WINDOW_HPP