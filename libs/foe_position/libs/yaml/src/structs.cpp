// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/position/yaml/structs.hpp>

#include <foe/ecs/yaml/id.hpp>
#include <foe/position/component/3d.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/glm.hpp>
#include <foe/yaml/pod.hpp>

bool yaml_read_foePosition3d(std::string const &nodeName,
                             YAML::Node const &node,
                             foePosition3d &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    foePosition3d newData = {};
    try {
        // glm::vec3 - position
        yaml_read_glm_vec3("position", readNode, newData.position);

        // glm::quat - orientation
        yaml_read_glm_quat("orientation", readNode, newData.orientation);
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

void yaml_write_foePosition3d(std::string const &nodeName,
                              foePosition3d const &data,
                              YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // glm::vec3 - position
        if (data.position != glm::vec3{}) {
            yaml_write_glm_vec3("position", data.position, writeNode);
        }

        // glm::quat - orientation
        if (data.orientation != glm::quat{}) {
            yaml_write_glm_quat("orientation", data.orientation, writeNode);
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
