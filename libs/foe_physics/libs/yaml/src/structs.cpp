// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/physics/yaml/structs.hpp>

#include <foe/ecs/yaml/id.hpp>
#include <foe/physics/component/rigid_body.h>
#include <foe/physics/resource/collision_shape_create_info.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/glm.hpp>
#include <foe/yaml/pod.hpp>

bool yaml_read_foeCollisionShapeCreateInfo(std::string const &nodeName,
                                           YAML::Node const &node,
                                           foeCollisionShapeCreateInfo &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    foeCollisionShapeCreateInfo newData = {};
    try {
        // glm::vec3 - boxSize
        yaml_read_glm_vec3("box_size", readNode, newData.boxSize);
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

void yaml_write_foeCollisionShapeCreateInfo(std::string const &nodeName,
                                            foeCollisionShapeCreateInfo const &data,
                                            YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // glm::vec3 - boxSize
        if (data.boxSize != glm::vec3{}) {
            yaml_write_glm_vec3("box_size", data.boxSize, writeNode);
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

bool yaml_read_foeRigidBody(std::string const &nodeName,
                            YAML::Node const &node,
                            foeEcsGroupTranslator groupTranslator,
                            foeRigidBody &data) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        return false;
    }

    foeRigidBody newData = {};
    try {
        // float - mass
        yaml_read_float("mass", readNode, newData.mass);

        // foeResourceID - collisionShape
        yaml_read_foeResourceID("collision_shape", readNode, groupTranslator,
                                newData.collisionShape);
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

void yaml_write_foeRigidBody(std::string const &nodeName,
                             foeRigidBody const &data,
                             YAML::Node &node) {
    YAML::Node writeNode;

    try {
        // float - mass
        if (data.mass != 0) {
            yaml_write_float("mass", data.mass, writeNode);
        }

        // foeResourceID - collisionShape
        if (data.collisionShape != FOE_INVALID_ID) {
            yaml_write_foeResourceID("collision_shape", data.collisionShape, writeNode);
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
