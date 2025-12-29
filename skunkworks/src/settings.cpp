// Copyright (C) 2021-2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "settings.hpp"

#include <foe/search_paths.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/pod.hpp>

#include "log.hpp"

#include <fstream>
#include <string>

namespace {

std::string outCfgFile = "out-foe-settings.yml";

bool parseEngineConfigFile(Settings *pOptions,
                           foeSearchPaths &searchPaths,
                           std::string_view configFilePath) {
    if (configFilePath.empty()) {
        FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_INFO, "Config file not found: {}", configFilePath);
        return true;
    }

    YAML::Node config;
    try {
        config = YAML::LoadFile(std::string{configFilePath});
    } catch (YAML::ParserException &e) {
        FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_FATAL, "Failed to load config file: {}", e.what());
        return false;
    }

    try {
        // General
        if (auto generalNode = config["general"]; generalNode) {
            try {
                yaml_read_bool("enableWindows", generalNode, pOptions->general.enableWindows);
            } catch (foeYamlException const &e) {
                throw foeYamlException{"general::" + e.whatStr()};
            }
        }

        // Window
        if (auto windowsListNode = config["windows"]; windowsListNode) {
            try {
                pOptions->windows.clear();
                for (auto it = windowsListNode.begin(); it != windowsListNode.end(); ++it) {
                    Settings::Window newWindow;

                    std::string implementation;
                    yaml_read_string("implementation", *it, implementation);
                    std::transform(implementation.begin(), implementation.end(),
                                   implementation.begin(),
                                   [](unsigned char c) { return std::tolower(c); });
                    if (implementation == "sdl3")
                        newWindow.implementation = Settings::Window::Implementation::SDL3;
                    else if (implementation == "glfw")
                        newWindow.implementation = Settings::Window::Implementation::GLFW;
                    else
                        // fallback is always GLFW
                        newWindow.implementation = Settings::Window::Implementation::GLFW;

                    yaml_read_string("title", *it, newWindow.title);

                    yaml_read_uint32_t("width", *it, newWindow.width);
                    yaml_read_uint32_t("height", *it, newWindow.height);

                    yaml_read_uint32_t("msaa", *it, newWindow.msaa);
                    yaml_read_bool("vsync", *it, newWindow.vsync);

                    pOptions->windows.emplace_back(std::move(newWindow));
                }
            } catch (foeYamlException const &e) {
                throw foeYamlException{"window::" + e.whatStr()};
            }
        }

        // Graphics
        if (auto graphicsNode = config["graphics"]; graphicsNode) {
            try {
                yaml_read_uint32_t("gpu", graphicsNode, pOptions->graphics.gpu);
                yaml_read_uint32_t("max_frame_buffering", graphicsNode,
                                   pOptions->graphics.maxFrameBuffering);
                yaml_read_bool("validation", graphicsNode, pOptions->graphics.validation);
                yaml_read_bool("debug_logging", graphicsNode, pOptions->graphics.debugLogging);
            } catch (foeYamlException const &e) {
                throw foeYamlException{"graphics::" + e.whatStr()};
            }
        }

        // Xr
        if (auto xrNode = config["xr"]; xrNode) {
            try {
                yaml_read_bool("enable", xrNode, pOptions->xr.enableXr);
                yaml_read_bool("force", xrNode, pOptions->xr.forceXr);
                yaml_read_bool("validation", xrNode, pOptions->xr.validation);
                yaml_read_bool("debug_logging", xrNode, pOptions->xr.debugLogging);
                yaml_read_uint32_t("msaa", xrNode, pOptions->xr.msaa);
            } catch (foeYamlException const &e) {
                throw foeYamlException{"xr::" + e.whatStr()};
            }
        }

        // Non-specific
        if (auto searchPathsNode = config["search_paths"]; searchPathsNode) {
            try {
                for (auto it = searchPathsNode.begin(); it != searchPathsNode.end(); ++it) {
                    std::string newPath;
                    yaml_read_string("", *it, newPath);

                    auto writer = searchPaths.getWriter();
                    writer.searchPaths()->emplace_back(std::move(newPath));
                }
            } catch (foeYamlException const &e) {
                throw foeYamlException{"search_paths::" + e.whatStr()};
            }
        }

    } catch (foeYamlException const &e) {
        FOE_LOG(foeSkunkworks, FOE_LOG_LEVEL_FATAL, "Failure parsing config file: {}", e.what());
        return false;
    }

    return true;
}

} // namespace

auto loadSettings(int argc, char **argv, Settings &settings, foeSearchPaths &searchPaths)
    -> std::tuple<bool, int> {
    std::string cfgFile = "foe-settings.yml";

    // Load settings from a configuration file (YAML)
    if (!parseEngineConfigFile(&settings, searchPaths, cfgFile)) {
        return std::make_tuple(false, 1);
    }

    return std::make_tuple(true, 0);
}