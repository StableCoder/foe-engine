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

#ifndef POSITION_3D_HPP
#define POSITION_3D_HPP

#include <foe/yaml/parsing.hpp>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <yaml-cpp/yaml.h>

struct Position3D {
    glm::vec3 position;
    glm::quat orientation;

    VkDescriptorSet descriptorSet{VK_NULL_HANDLE};
};

inline auto yaml_read_Position3D(YAML::Node const &node) -> Position3D {
    Position3D position;

    yaml_read_required("position", node, position.position);
    yaml_read_required("orientation", node, position.orientation);

    return position;
}

inline auto yaml_write_Position3D(Position3D const &data) -> YAML::Node {
    YAML::Node writeNode;

    yaml_write_required("position", data.position, writeNode);
    yaml_write_required("orientation", data.orientation, writeNode);

    return writeNode;
}

#endif // POSITION_3D_HPP