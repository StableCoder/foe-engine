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

#ifndef ENGINE_SETTINGS_HPP
#define ENGINE_SETTINGS_HPP

#include <CLI/CLI11.hpp>
#include <yaml-cpp/yaml.h>

#include <string>
#include <string_view>
#include <vector>

/// Settings related to the core engine
struct EngineSettings {
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
        bool forceXr = false;
        bool debugLogging = false;
    } xr;
};

void addEngineCommandLineOptions(CLI::App *pParser, EngineSettings *pOptions);

bool parseEngineConfigFile(EngineSettings *pOptions, std::string_view configFilePath);

void emitEngineSettingsYaml(EngineSettings const *pOptions, YAML::Node *pNode);

#endif // ENGINE_SETTINGS_HPP