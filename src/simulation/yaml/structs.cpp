// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "structs.hpp"

#include <foe/ecs/yaml/id.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/pod.hpp>

#include "../armature_create_info.h"
#include "../armature_state.hpp"
#include "../cleanup.h"
#include "../render_state.h"
#include "../type_defs.h"

#include <string.h>

bool yaml_read_AnimationImportInfo(std::string const &nodeName,
                                   YAML::Node const &node,
                                   AnimationImportInfo &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    AnimationImportInfo newData = {};
    try {
        // char const * - pFile[null-terminated]
        if (std::string pFile; yaml_read_string("file", readNode, pFile)) {
            newData.pFile = (char *)malloc(pFile.size() + 1);
            memcpy((char *)newData.pFile, pFile.c_str(), pFile.size() + 1);
        }

        // char const * - pName[null-terminated]
        if (std::string pName; yaml_read_string("name", readNode, pName)) {
            newData.pName = (char *)malloc(pName.size() + 1);
            memcpy((char *)newData.pName, pName.c_str(), pName.size() + 1);
        }
    } catch (foeYamlException const &e) {
        cleanup_AnimationImportInfo(&newData);

        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_AnimationImportInfo(std::string const &nodeName,
                                    AnimationImportInfo const &data,
                                    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // char const * - pFile[null-terminated]
        if (data.pFile) {
            yaml_write_string("file", data.pFile, writeNode);
        }

        // char const * - pName[null-terminated]
        if (data.pName) {
            yaml_write_string("name", data.pName, writeNode);
        }
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

bool yaml_read_foeArmatureCreateInfo(std::string const &nodeName,
                                     YAML::Node const &node,
                                     foeArmatureCreateInfo &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    foeArmatureCreateInfo newData = {};
    try {
        // char const * - pFile[null-terminated]
        if (std::string pFile; yaml_read_string("file", readNode, pFile)) {
            newData.pFile = (char *)malloc(pFile.size() + 1);
            memcpy((char *)newData.pFile, pFile.c_str(), pFile.size() + 1);
        }

        // char const * - pRootArmatureNode[null-terminated]
        if (std::string pRootArmatureNode;
            yaml_read_string("root_armature_node", readNode, pRootArmatureNode)) {
            newData.pRootArmatureNode = (char *)malloc(pRootArmatureNode.size() + 1);
            memcpy((char *)newData.pRootArmatureNode, pRootArmatureNode.c_str(),
                   pRootArmatureNode.size() + 1);
        }

        // AnimationImportInfo* - pAnimations[animationCount]
        if (YAML::Node animations_node = readNode["animations"]; animations_node) {
            // Set the associated control member
            newData.animationCount = animations_node.size();

            if (newData.animationCount > 0) {
                newData.pAnimations = (AnimationImportInfo *)malloc(newData.animationCount *
                                                                    sizeof(AnimationImportInfo));
                for (size_t i = 0; i < newData.animationCount; ++i) {
                    YAML::Node subReadNode = animations_node[i];
                    if (!yaml_read_AnimationImportInfo("", subReadNode, newData.pAnimations[i])) {
                        throw foeYamlException{"animations - Failed to read list-node"};
                    }
                }
            }
        }
    } catch (foeYamlException const &e) {
        cleanup_foeArmatureCreateInfo(&newData);

        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_foeArmatureCreateInfo(std::string const &nodeName,
                                      foeArmatureCreateInfo const &data,
                                      YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // char const * - pFile[null-terminated]
        if (data.pFile) {
            yaml_write_string("file", data.pFile, writeNode);
        }

        // char const * - pRootArmatureNode[null-terminated]
        if (data.pRootArmatureNode) {
            yaml_write_string("root_armature_node", data.pRootArmatureNode, writeNode);
        }

        // AnimationImportInfo* - pAnimations[animationCount]
        if (data.animationCount > 0) {
            YAML::Node subWriteNode;

            for (size_t i = 0; i < data.animationCount; ++i) {
                YAML::Node listNode;
                yaml_write_AnimationImportInfo("", data.pAnimations[i], listNode);
                subWriteNode.push_back(listNode);
            }

            writeNode["animations"] = subWriteNode;
        }
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

bool yaml_read_foeArmatureState(std::string const &nodeName,
                                YAML::Node const &node,
                                foeEcsGroupTranslator groupTranslator,
                                foeArmatureState &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    foeArmatureState newData = {};
    try {
        // foeResourceID - armatureID
        yaml_read_foeResourceID("armature_id", readNode, groupTranslator, newData.armatureID);

        // uint32_t - animationID
        yaml_read_uint32_t("animation_id", readNode, newData.animationID);

        // float - time
        yaml_read_float("time", readNode, newData.time);
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_foeArmatureState(std::string const &nodeName,
                                 foeArmatureState const &data,
                                 YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // foeResourceID - armatureID
        if (data.armatureID != FOE_INVALID_ID) {
            yaml_write_foeResourceID("armature_id", data.armatureID, writeNode);
        }

        // uint32_t - animationID
        if (data.animationID != 0) {
            yaml_write_uint32_t("animation_id", data.animationID, writeNode);
        }

        // float - time
        if (data.time != 0) {
            yaml_write_float("time", data.time, writeNode);
        }
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

bool yaml_read_foeRenderState(std::string const &nodeName,
                              YAML::Node const &node,
                              foeEcsGroupTranslator groupTranslator,
                              foeRenderState &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    foeRenderState newData = {};
    try {
        // foeResourceID - vertexDescriptor
        yaml_read_foeResourceID("vertex_descriptor", readNode, groupTranslator,
                                newData.vertexDescriptor);

        // foeResourceID - bonedVertexDescriptor
        yaml_read_foeResourceID("boned_vertex_descriptor", readNode, groupTranslator,
                                newData.bonedVertexDescriptor);

        // foeResourceID - material
        yaml_read_foeResourceID("material", readNode, groupTranslator, newData.material);

        // foeResourceID - mesh
        yaml_read_foeResourceID("mesh", readNode, groupTranslator, newData.mesh);
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    data = newData;
    return true;
}

void yaml_write_foeRenderState(std::string const &nodeName,
                               foeRenderState const &data,
                               YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // foeResourceID - vertexDescriptor
        if (data.vertexDescriptor != FOE_INVALID_ID) {
            yaml_write_foeResourceID("vertex_descriptor", data.vertexDescriptor, writeNode);
        }

        // foeResourceID - bonedVertexDescriptor
        if (data.bonedVertexDescriptor != FOE_INVALID_ID) {
            yaml_write_foeResourceID("boned_vertex_descriptor", data.bonedVertexDescriptor,
                                     writeNode);
        }

        // foeResourceID - material
        if (data.material != FOE_INVALID_ID) {
            yaml_write_foeResourceID("material", data.material, writeNode);
        }

        // foeResourceID - mesh
        if (data.mesh != FOE_INVALID_ID) {
            yaml_write_foeResourceID("mesh", data.mesh, writeNode);
        }
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.what()};
        }
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}
