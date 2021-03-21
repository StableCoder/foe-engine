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

#include <foe/resource/yaml/armature.hpp>

#include <foe/log.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>
#include <yaml-cpp/yaml.h>

bool import_yaml_armature_definition(std::string_view armatureName,
                                     std::string &fileName,
                                     std::string &rootArmatureNode,
                                     std::vector<AnimationImportInfo> &animations) {
    // Open the YAML file
    YAML::Node rootNode;
    try {
        rootNode = YAML::LoadFile(std::string{armatureName} + ".yml");
    } catch (YAML::ParserException &e) {
        FOE_LOG(General, Fatal, "Failed to load Yaml file: {}", e.what());
    }

    try {
        // Read the definition
        {
            // Sub-Resources

            // Data
            if (auto dataNode = rootNode["data"]; dataNode) {
                try {
                    yaml_read_required("fileName", dataNode, fileName);
                    yaml_read_required("root_armature_node", dataNode, rootArmatureNode);

                    if (auto animationsNode = dataNode["animations"]; animationsNode) {
                        for (auto it = animationsNode.begin(); it != animationsNode.end(); ++it) {
                            AnimationImportInfo animation;

                            yaml_read_required("fileName", *it, animation.file);
                            yaml_read_required("name", *it, animation.animationName);

                            animations.emplace_back(animation);
                        }
                    } else {
                        throw foeYamlException("animations - Required node not found");
                    }

                } catch (foeYamlException const &e) {
                    throw foeYamlException("data::" + e.whatStr());
                }
            } else {
                throw foeYamlException("Required 'data' node not found.");
            }
        }

    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to import foeArmature definition: {}", e.what());
        return false;
    }

    return true;
}