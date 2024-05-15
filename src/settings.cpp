// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "settings.hpp"

#include <CLI/CLI11.hpp>
#include <foe/search_paths.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/pod.hpp>

#include "log.hpp"

#include <fstream>
#include <string>

namespace {

std::string outCfgFile = "out-foe-settings.yml";

void addCommandLineOptions(CLI::App *pParser, Settings *pOptions, foeSearchPaths &searchPaths) {
    // Window
    pParser->add_flag("--window,!--no-window", pOptions->window.enableWSI,
                      "Whether or not to start with an initial window");
    pParser->add_option("--wsi-implementation", pOptions->window.implementation,
                        "Path to the WSI implementation library");
    pParser->add_option("--width", pOptions->window.width, "Width of the initial window");
    pParser->add_option("--height", pOptions->window.height, "Height of the initial window");
    pParser->add_flag("--vsync,!--no-vsync", pOptions->window.vsync, "V-Sync");

    // Graphics
    pParser->add_option("--gpu", pOptions->graphics.gpu, "Physical GPU to use to render");
    pParser->add_option("--frame-buffering", pOptions->graphics.maxFrameBuffering,
                        "Maximum frames to buffer");
    pParser->add_option("--msaa", pOptions->graphics.msaa, "MSAA to use (typically 1,2,4 or 8)");
    pParser->add_flag("--gfx-validation", pOptions->graphics.validation,
                      "Turns on graphics validation layers");
    pParser->add_flag("--gfx-debug-logging", pOptions->graphics.debugLogging,
                      "Turns on the graphics debug logging callback");

    // Xr
    pParser->add_flag("--vr,!--no-vr", pOptions->xr.enableXr, "Enable VR (OpenXR)");
    pParser->add_flag("--force-vr", pOptions->xr.forceXr, "Force using VR (Fail without it)");
    pParser->add_flag("--vr-validation", pOptions->xr.validation, "Turns on vr validation layers");
    pParser->add_option("--vr-debug-logging", pOptions->xr.debugLogging,
                        "Turns on OpenXR debug logging");

    // Non-Specific
    pParser->add_option("--dump-config", outCfgFile,
                        "If specified, the config on exit will be written to this file (default: "
                        "out-foe-settings.yml)");

    pParser->add_option_function<std::string>(
        "--search-path",
        [&](std::string const &data) {
            auto writer = searchPaths.getWriter();
            writer.searchPaths()->push_back(data);
        },
        "Adds a path that the program uses to search for data sets");
}

bool parseEngineConfigFile(Settings *pOptions,
                           foeSearchPaths &searchPaths,
                           std::string_view configFilePath) {
    if (configFilePath.empty()) {
        FOE_LOG(foeBringup, FOE_LOG_LEVEL_INFO, "Config file not found: {}", configFilePath);
        return true;
    }

    YAML::Node config;
    try {
        config = YAML::LoadFile(std::string{configFilePath});
    } catch (YAML::ParserException &e) {
        FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "Failed to load config file: {}", e.what());
        return false;
    }

    try {
        // Window
        if (auto windowNode = config["window"]; windowNode) {
            try {
                yaml_read_bool("have_window", windowNode, pOptions->window.enableWSI);
                yaml_read_string("implementation", windowNode, pOptions->window.implementation);
                yaml_read_uint32_t("width", windowNode, pOptions->window.width);
                yaml_read_uint32_t("height", windowNode, pOptions->window.height);
                yaml_read_bool("vsync", windowNode, pOptions->window.vsync);
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
                yaml_read_uint32_t("msaa", graphicsNode, pOptions->graphics.msaa);
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
                throw foeYamlException{"search_paths" + e.whatStr()};
            }
        }

    } catch (foeYamlException const &e) {
        FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "Failure parsing config file: {}", e.what());
        return false;
    }

    return true;
}

void emitSettingsYaml(Settings const *pOptions, YAML::Node *pNode) {
    Settings defaultOptions;

    try {
        { // Window
            bool writeNode{false};
            YAML::Node windowNode;

            if (defaultOptions.window.width != pOptions->window.width) {
                yaml_write_uint32_t("width", pOptions->window.width, windowNode);
                writeNode = true;
            }
            if (defaultOptions.window.height != pOptions->window.height) {
                yaml_write_uint32_t("height", pOptions->window.height, windowNode);
                writeNode = true;
            }
            if (defaultOptions.window.vsync != pOptions->window.vsync) {
                yaml_write_bool("vsync", pOptions->window.vsync, windowNode);
                writeNode = true;
            }

            if (writeNode) {
                (*pNode)["window"] = windowNode;
            }
        }

        { // Graphics
            bool writeNode{false};
            YAML::Node graphicsNode;

            if (defaultOptions.graphics.gpu != pOptions->graphics.gpu) {
                yaml_write_uint32_t("gpu", pOptions->graphics.gpu, graphicsNode);
                writeNode = true;
            }
            if (defaultOptions.graphics.maxFrameBuffering != pOptions->graphics.maxFrameBuffering) {
                yaml_write_uint32_t("max_frame_buffering", pOptions->graphics.maxFrameBuffering,
                                    graphicsNode);
                writeNode = true;
            }
            if (defaultOptions.graphics.msaa != pOptions->graphics.msaa) {
                yaml_write_uint32_t("msaa", pOptions->graphics.msaa, graphicsNode);
                writeNode = true;
            }

            if (writeNode) {
                (*pNode)["graphics"] = graphicsNode;
            }
        }
    } catch (foeYamlException const &e) {
        FOE_LOG(foeBringup, FOE_LOG_LEVEL_FATAL, "Failed to write Yaml engine options: {}",
                e.what());
    }
}

} // namespace

auto loadSettings(int argc,
                  char **argv,
                  Settings &settings,
                  foeSearchPaths &searchPaths) -> std::tuple<bool, int> {
    std::string cfgFile = "foe-settings.yml";

    { // Load settings from command line
        CLI::App clParser{"This is the FoE Engine Bringup Application"};

        clParser.add_option("--config", cfgFile,
                            "Configuration file to load settings from (default: foe-settings.yml)");

        try {
            (clParser).parse((argc), (argv));
        } catch (const CLI::ParseError &e) {
            if (e.get_name() != "CallForHelp") {
                return std::make_tuple(false, (clParser).exit(e));
            }
        }

        { // Load settings from a configuration file (YAML)
            if (!parseEngineConfigFile(&settings, searchPaths, cfgFile)) {
                return std::make_tuple(false, 1);
            }
        }

        addCommandLineOptions(&clParser, &settings, searchPaths);

        try {
            (clParser).parse((argc), (argv));
        } catch (const CLI::ParseError &e) {
            return std::make_tuple(false, (clParser).exit(e));
        }
    }

    return std::make_tuple(true, 0);
}

bool saveSettings(Settings const &settings) {
    YAML::Node yamlSettings;

    emitSettingsYaml(&settings, &yamlSettings);

    YAML::Emitter emitter;
    emitter << yamlSettings;

    std::ofstream outFile(outCfgFile, std::ofstream::out);
    if (outFile.is_open()) {
        outFile << emitter.c_str();
        outFile.close();
        return true;
    } else {
        return false;
    }
}