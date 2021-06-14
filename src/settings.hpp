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

#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <cstdint>
#include <tuple>

class foeSearchPaths;

struct Settings {
    struct Window {
        bool enableWSI = true;
        uint32_t width = 1280;
        uint32_t height = 720;
        bool vsync = false;
    } window;

    struct Graphics {
        uint32_t gpu = UINT32_MAX;
        uint32_t maxFrameBuffering = UINT32_MAX;
        bool validation = false;
        bool debugLogging = false;
    } graphics;

    struct Xr {
        bool enableXr = true;
        bool forceXr = false;
        bool validation = false;
        bool debugLogging = false;
    } xr;
};

/// Returns two items, a boolean on whether to continue the program, and code to return with.
auto loadSettings(int argc, char **argv, Settings &settings, foeSearchPaths &searchPaths)
    -> std::tuple<bool, int>;

bool saveSettings(Settings const &settings);

#endif // SETTINGS_HPP