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

#ifndef IMGUI_FRAME_TIME_INFO_HPP
#define IMGUI_FRAME_TIME_INFO_HPP

#include <foe/imgui/base.hpp>

class FrameTimer;

class foeImGuiFrameTimeInfo : public foeImGuiBase {
  public:
    foeImGuiFrameTimeInfo(FrameTimer *pFrameTimer);

  private:
    void viewMainMenu() final;
    void customUI() final;

    FrameTimer const *const mFrameTimer;

    bool mOpen{true};
};

#endif // IMGUI_FRAME_TIME_INFO_HPP