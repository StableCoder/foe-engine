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

#include <foe/ecs/yaml/index_generator.hpp>

#include <foe/yaml/exception.hpp>

void yaml_read_index_generator(YAML::Node const &node, foeIdIndexGenerator &indexGenerator) {
    foeIdIndex nextIndex;
    std::vector<foeIdIndex> recycledIndices;

    // Next Free Index
    if (auto nextNode = node["next_free_index"]; nextNode) {
        try {
            nextIndex = nextNode.as<foeIdIndex>();
        } catch (...) {
            throw foeYamlException{
                "yaml_read_index_generator::next_free_index - Could not parse value of '" +
                nextNode.as<std::string>() + "' to foeIdIndex"};
        }
    } else {
        throw foeYamlException{
            "yaml_read_index_generator - Could not find required 'next_free_index' node"};
    }

    // Recycled Indices
    if (auto recycledNode = node["recycled_indices"]; recycledNode) {
        recycledIndices.reserve(recycledNode.size());

        for (auto it = recycledNode.begin(); it != recycledNode.end(); ++it) {
            try {
                recycledIndices.emplace_back(it->as<foeIdIndex>());
            } catch (...) {
                throw foeYamlException{
                    "yaml_read_index_generator::recycled_indices - Could not parse value of '" +
                    it->as<std::string>() + "' as a foeIdIndex"};
            }
        }
    } else {
        throw foeYamlException{
            "yaml_read_index_generator - Could not find required 'recycled_indices' node"};
    }

    indexGenerator.importState(nextIndex, recycledIndices);
}

auto yaml_write_index_generator(foeIdIndexGenerator &data) -> YAML::Node {
    YAML::Node node;

    foeIdIndex nextIndex;
    std::vector<foeIdIndex> recycledIndices;

    data.exportState(nextIndex, recycledIndices);

    // Next Free Index
    node["next_free_index"] = nextIndex;

    // Recycled Indices
    YAML::Node recycledNode;
    for (auto it : recycledIndices) {
        recycledNode.push_back(it);
    }
    node["recycled_indices"] = recycledNode;

    return node;
}