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

bool yaml_read_armature_definition(YAML::Node const &node,
                                   foeIdGroupTranslator const *pTranslator,
                                   foeArmatureCreateInfo &createInfo) {
    // Sub-Resources

    // Data
    if (auto dataNode = node["data"]; dataNode) {
        try {
            yaml_read_required("fileName", dataNode, createInfo.fileName);
            yaml_read_required("root_armature_node", dataNode, createInfo.rootArmatureNode);

            if (auto animationsNode = dataNode["animations"]; animationsNode) {
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
            throw foeYamlException("data::" + e.whatStr());
        }
    } else {
        throw foeYamlException("Required 'data' node not found.");
    }

    return true;
}

void yaml_read_armature_definition2(YAML::Node const &node,
                                    foeIdGroupTranslator const *pTranslator,
                                    foeResourceCreateInfoBase **ppCreateInfo) {
    foeArmatureCreateInfo ci;

    yaml_read_armature_definition(node, pTranslator, ci);

    *ppCreateInfo = new foeArmatureCreateInfo(std::move(ci));
}