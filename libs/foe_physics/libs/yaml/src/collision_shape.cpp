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

#include "collision_shape.hpp"

#include <foe/physics/resource/collision_shape_loader.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include <string_view>

namespace {

bool yaml_read_collision_shape_definition_internal(std::string const &nodeName,
                                                   YAML::Node const &node,
                                                   foeIdGroupTranslator const *pTranslator,
                                                   foeCollisionShapeCreateInfo &createInfo) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        yaml_read_required("box_size", subNode, createInfo.boxSize);
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.whatStr()};
        }
    }

    return true;
}

void yaml_write_collision_shape_definition_internal(std::string const &nodeName,
                                                    foeCollisionShapeCreateInfo const &data,
                                                    YAML::Node &node) {
    YAML::Node writeNode;

    try {
        yaml_write_required("box_size", data.boxSize, writeNode);
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

char const *yaml_collision_shape_key() { return "collision_shape_v1"; }

void yaml_read_collision_shape(YAML::Node const &node,
                               foeIdGroupTranslator const *pTranslator,
                               foeResourceCreateInfoBase **ppCreateInfo) {
    foeCollisionShapeCreateInfo ci;

    yaml_read_collision_shape_definition_internal(yaml_collision_shape_key(), node, pTranslator,
                                                  ci);

    *ppCreateInfo = new foeCollisionShapeCreateInfo(std::move(ci));
}

auto yaml_write_collision_shape(foeCollisionShapeCreateInfo &data) -> YAML::Node {
    YAML::Node outNode;

    yaml_write_collision_shape_definition_internal("", data, outNode);

    return outNode;
}