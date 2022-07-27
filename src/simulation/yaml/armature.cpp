// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "armature.hpp"

#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include "../armature_create_info.hpp"
#include "../type_defs.h"

namespace {

bool yaml_read_armature_definition_internal(std::string const &nodeName,
                                            YAML::Node const &node,
                                            foeEcsGroupTranslator groupTranslator,
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
            createInfo.animationSetCount = animationsNode.size();

            createInfo.pAnimationSets = new AnimationImportInfo[createInfo.animationSetCount];

            size_t animSetCount = 0;
            for (auto it = animationsNode.begin(); it != animationsNode.end(); ++it) {
                AnimationImportInfo animation;

                yaml_read_required("fileName", *it, animation.file);

                if (auto animationNamesNode = (*it)["animationNames"]; animationNamesNode) {
                    animation.animationNameCount = animationNamesNode.size();

                    animation.pAnimationNames = new std::string[animation.animationNameCount];

                    size_t animNameCount = 0;
                    for (auto it = animationNamesNode.begin(); it != animationNamesNode.end();
                         ++it) {
                        std::string tempStr;

                        yaml_read_required("", *it, tempStr);

                        animation.pAnimationNames[animNameCount] = std::move(tempStr);
                        ++animNameCount;
                    }
                }

                createInfo.pAnimationSets[animSetCount] = animation;
                ++animSetCount;
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

            for (uint32_t i = 0; i < data.animationSetCount; ++i) {
                auto const &it = data.pAnimationSets[i];
                YAML::Node animationListNode;
                for (uint32_t j = 0; j < it.animationNameCount; ++j) {
                    auto const &subIt = it.pAnimationNames[j];
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

char const *yaml_armature_key() { return "armature_v1"; }

void yaml_read_armature(YAML::Node const &node,
                        foeEcsGroupTranslator groupTranslator,
                        foeResourceCreateInfo *pCreateInfo) {
    foeArmatureCreateInfo armatureCI{};
    foeResourceCreateInfo createInfo;

    yaml_read_armature_definition_internal(yaml_armature_key(), node, groupTranslator, armatureCI);

    auto dataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (foeArmatureCreateInfo *)pSrc;
        new (pDst) foeArmatureCreateInfo(std::move(*pSrcData));
    };

    foeResultSet result = foeCreateResourceCreateInfo(
        FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_CREATE_INFO, foeDestroyArmatureCreateInfo,
        sizeof(foeArmatureCreateInfo), &armatureCI, dataFn, &createInfo);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        throw foeYamlException{
            std::string{"Failed to create foeArmatureCreateInfo due to error: "} + buffer};
    }

    *pCreateInfo = createInfo;
}

auto yaml_write_armature(foeArmatureCreateInfo const &data) -> YAML::Node {
    YAML::Node outNode;

    yaml_write_armature_internal("", data, outNode);

    return outNode;
}