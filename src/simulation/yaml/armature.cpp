// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "armature.hpp"

#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include "../armature_create_info.h"
#include "../cleanup.h"
#include "../type_defs.h"

#include <string.h>

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
        std::string tempStr;

        yaml_read_required("fileName", subNode, tempStr);
        createInfo.pFile = (char *)malloc(tempStr.size() + 1);
        memcpy((char *)createInfo.pFile, tempStr.c_str(), tempStr.size() + 1);

        yaml_read_required("root_armature_node", subNode, tempStr);
        createInfo.pRootArmatureNode = (char *)malloc(tempStr.size() + 1);
        memcpy((char *)createInfo.pRootArmatureNode, tempStr.c_str(), tempStr.size() + 1);

        if (auto animationsNode = subNode["animations"]; animationsNode) {
            createInfo.animationCount = animationsNode.size();

            createInfo.pAnimations = (AnimationImportInfo *)malloc(createInfo.animationCount *
                                                                   sizeof(AnimationImportInfo));

            size_t animSetCount = 0;

            for (auto it = animationsNode.begin(); it != animationsNode.end(); ++it) {
                AnimationImportInfo animation = {};

                std::string tempStr;
                yaml_read_required("fileName", *it, tempStr);
                animation.pFile = (char *)malloc(tempStr.size() + 1);
                memcpy((void *)animation.pFile, tempStr.c_str(), tempStr.size() + 1);

                yaml_read_required("animationName", *it, tempStr);
                animation.pName = (char *)malloc(tempStr.size() + 1);
                memcpy((void *)animation.pName, tempStr.c_str(), tempStr.size() + 1);

                createInfo.pAnimations[animSetCount] = animation;
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
        yaml_write_required("fileName", std::string{data.pFile}, writeNode);
        yaml_write_required("root_armature_node", std::string{data.pRootArmatureNode}, writeNode);

        { // Animation Data
            YAML::Node animationsNode;

            for (uint32_t i = 0; i < data.animationCount; ++i) {
                auto const &it = data.pAnimations[i];
                YAML::Node animationNode;

                yaml_write_required("fileName", std::string{it.pFile}, animationNode);
                yaml_write_required("animationName", std::string{it.pName}, animationNode);

                animationsNode.push_back(animationNode);
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
        FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_CREATE_INFO,
        (PFN_foeResourceCreateInfoCleanup)cleanup_foeArmatureCreateInfo,
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