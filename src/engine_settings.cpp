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

#include "engine_settings.hpp"

#include <foe/log.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

void addEngineCommandLineOptions(CLI::App *pParser, EngineSettings *pOptions) {
    // Window
    pParser->add_flag("--window,!--no-window", pOptions->window.enableWSI,
                      "Whether or not to start with an initial window");
    pParser->add_option("--width", pOptions->window.width, "Width of the initial window");
    pParser->add_option("--height", pOptions->window.height, "Height of the initial window");
    pParser->add_flag("--vsync,!--no-vsync", pOptions->window.vsync, "V-Sync");

    // Graphics
    pParser->add_option("--gpu", pOptions->graphics.gpu, "Physical GPU to use to render");
    pParser->add_option("--frame-buffering", pOptions->graphics.maxFrameBuffering,
                        "Maximum frames to buffer");
    pParser->add_flag("--gfx-validation", pOptions->graphics.validation,
                      "Turns on graphics validation layers");
    pParser->add_flag("--gfx-debug-logging", pOptions->graphics.debugLogging,
                      "Turns on the graphics debug logging callback");

    // Xr
    pParser->add_flag("--vr,!--no-vr", pOptions->xr.forceXr, "VR (OpenXR)");
    pParser->add_option("--vr-debug-logging", pOptions->xr.debugLogging,
                        "Turns on OpenXR debug logging");
}

bool parseEngineConfigFile(EngineSettings *pOptions, std::string_view configFilePath) {
    if (configFilePath.empty()) {
        FOE_LOG(General, Info, "No config file found");
        return true;
    }

    YAML::Node config;
    try {
        config = YAML::LoadFile(std::string{configFilePath});
    } catch (YAML::ParserException &e) {
        FOE_LOG(General, Fatal, "Failed to load config file: {}", e.what());
        return false;
    }

    try {
        // Window
        if (auto windowNode = config["window"]; windowNode) {
            yaml_read_optional("have_window", windowNode, pOptions->window.enableWSI);
            yaml_read_optional("width", windowNode, pOptions->window.width);
            yaml_read_optional("height", windowNode, pOptions->window.height);
            yaml_read_optional("vsync", windowNode, pOptions->window.vsync);
        }

        // Graphics
        if (auto graphicsNode = config["graphics"]; graphicsNode) {
            yaml_read_optional("gpu", graphicsNode, pOptions->graphics.gpu);
            yaml_read_optional("max_frame_buffering", graphicsNode,
                               pOptions->graphics.maxFrameBuffering);
            yaml_read_optional("validation", graphicsNode, pOptions->graphics.validation);
            yaml_read_optional("debug_logging", graphicsNode, pOptions->graphics.debugLogging);
        }

        // Xr
        if (auto xrNode = config["xr"]; xrNode) {
            yaml_read_optional("xr", xrNode, pOptions->xr.forceXr);
            yaml_read_optional("debug_logging", xrNode, pOptions->xr.debugLogging);
        }

    } catch (foeYamlException const &e) {
        FOE_LOG(General, Fatal, "Failure parsing config file: {}", e.what());
        return false;
    }

    return true;
}

void emitEngineSettingsYaml(EngineSettings const *pOptions, YAML::Node *pNode) {
    EngineSettings defaultOptions;

    try {
        { // Window
            bool writeNode{false};
            YAML::Node windowNode;

            writeNode |= yaml_write_optional("width", defaultOptions.window.width,
                                             pOptions->window.width, windowNode);
            writeNode |= yaml_write_optional("height", defaultOptions.window.height,
                                             pOptions->window.height, windowNode);
            writeNode |= yaml_write_optional("vsync", defaultOptions.window.vsync,
                                             pOptions->window.vsync, windowNode);

            if (writeNode) {
                (*pNode)["window"] = windowNode;
            }
        }

        { // Graphics
            bool writeNode{false};
            YAML::Node graphicsNode;

            writeNode |= yaml_write_optional("gpu", defaultOptions.graphics.gpu,
                                             pOptions->graphics.gpu, graphicsNode);
            writeNode |= yaml_write_optional("max_frame_buffering",
                                             defaultOptions.graphics.maxFrameBuffering,
                                             pOptions->graphics.maxFrameBuffering, graphicsNode);

            if (writeNode) {
                (*pNode)["graphics"] = graphicsNode;
            }
        }
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Fatal, "Failed to write Yaml engine options: {}", e.what());
    }
}