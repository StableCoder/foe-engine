// Copyright (C) 2021-2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <cstdint>
#include <string>
#include <vector>

class foeSearchPaths;

struct Settings {
    struct General {
        bool enableWindows = true;
    } general;

    struct Window {
        enum Implementation {
            GLFW,
        };

        Implementation implementation = Implementation::GLFW;
        std::string title = "FoE Skunkworks";
        uint32_t width = 1280;
        uint32_t height = 720;
        bool vsync = false;
    };
    std::vector<Window> windows{Window()};

    struct Graphics {
        uint32_t gpu = UINT32_MAX;
        uint32_t msaa = 1; // By default no MSAA
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

#endif // SETTINGS_HPP