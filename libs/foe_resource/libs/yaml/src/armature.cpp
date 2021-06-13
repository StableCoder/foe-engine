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

#include "armature.hpp"

#include <foe/log.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

namespace {

constexpr std::string_view cNodeName = "armature_v1";

bool yaml_read_armature_definition_internal(std::string const &nodeName,
                                            YAML::Node const &node,
                                            foeIdGroupTranslator const *pTranslator,
                                            foeArmatureCreateInfo &createInfo) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        // Data
        yaml_read_required("fileName", subNode, createInfo.fileName);
        yaml_read_required("root_armature_node", subNode, createInfo.rootArmatureNode);

        if (auto animationsNode = subNode["animations"]; animationsNode) {
            for (auto it = animationsNode.begin(); it != animationsNode.end(); ++it) {
                AnimationImportInfo animation;

                yaml_read_required("fileName", *it, animation.file);

                if (auto animationNamesNode = (*it)["animationNames"]; animationNamesNode) {
                    for (auto it = animationNamesNode.begin(); it != animationNamesNode.end();
                         ++it) {
                        std::string tempStr;

                        yaml_read_required("", *it, tempStr);

                        animation.animationNames.push_back(std::move(tempStr));
                    }
                }

                createInfo.animations.emplace_back(animation);
            }
        } else {
            throw foeYamlException("animations - Required node not found");
        }
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.whatStr()};
        }
    }

    return true;
}

void yaml_write_armature_internal(std::string const &nodeName,
                                  foeArmatureCreateInfo const &data,
                                  YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // Armature Data
        yaml_write_required("fileName", data.fileName, writeNode);
        yaml_write_required("root_armature_node", data.rootArmatureNode, writeNode);

        { // Animation Data
            YAML::Node animationsNode;

            for (auto const &it : data.animations) {
                YAML::Node animationListNode;
                for (auto const &subIt : it.animationNames) {
                    YAML::Node animationNode;

                    yaml_write_required("", subIt, animationNode);

                    animationListNode.push_back(animationNode);
                }

                YAML::Node animationFileNode;
                yaml_write_required("fileName", it.file, animationFileNode);
                animationFileNode["animationNames"] = animationListNode;

                animationsNode.push_back(animationFileNode);
            }

            writeNode["animations"] = animationsNode;
        }
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.whatStr()};
        }
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

} // namespace

void yaml_read_armature_definition(YAML::Node const &node,
                                   foeIdGroupTranslator const *pTranslator,
                                   foeResourceCreateInfoBase **ppCreateInfo) {
    foeArmatureCreateInfo ci;

    yaml_read_armature_definition_internal(std::string{cNodeName}, node, pTranslator, ci);

    *ppCreateInfo = new foeArmatureCreateInfo(std::move(ci));
}

auto yaml_write_armature_definition(foeArmatureCreateInfo &data) -> YAML::Node {
    YAML::Node outNode;

    yaml_write_armature_internal("", data, outNode);

    return outNode;
}