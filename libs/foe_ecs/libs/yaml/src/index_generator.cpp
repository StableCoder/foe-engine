/*
    Copyright (C) 2021-2022 George Cave.

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
#include <foe/yaml/parsing.hpp>

void yaml_read_index_generator(std::string const &nodeName,
                               YAML::Node const &node,
                               foeIdIndexGenerator &indexGenerator) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        throw foeYamlException(nodeName + " - Required node to parse not found");
    }

    foeIdIndex nextIndex;
    std::vector<foeIdIndex> recycledIndices;

    // Next Free Index
    try {
        yaml_read_required("next_free_index", readNode, nextIndex);
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw foeYamlException{e.whatStr()};
        } else {
            throw foeYamlException{nodeName + "::" + e.whatStr()};
        }
    }

    // Recycled Indices
    try {
        if (auto recycledNode = readNode["recycled_indices"]; recycledNode) {
            recycledIndices.reserve(recycledNode.size());

            for (auto it = recycledNode.begin(); it != recycledNode.end(); ++it) {
                foeIdIndex readIndex;
                yaml_read_required("", *it, readIndex);
                recycledIndices.emplace_back(readIndex);
            }
        } else {
            throw foeYamlException{" - Required node not found"};
        }
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw foeYamlException{"recycled_indices" + e.whatStr()};
        } else {
            throw foeYamlException{nodeName + "::recycled_indices" + e.whatStr()};
        }
    }

    indexGenerator.importState(nextIndex, recycledIndices.size(), recycledIndices.data());
}

void yaml_write_index_generator(std::string const &nodeName,
                                foeIdIndexGenerator &data,
                                YAML::Node &node) {
    YAML::Node newNode;
    YAML::Node *pWriteNode{nullptr};
    if (nodeName.empty()) {
        pWriteNode = &node;
    } else {
        pWriteNode = &newNode;
        if (auto existingNode = node[nodeName]; existingNode) {
            newNode = existingNode;
        }
    }

    foeResult result;
    foeIdIndex nextIndex;
    std::vector<foeIdIndex> recycledIndices;

    do {
        uint32_t recycledCount;
        data.exportState(nullptr, &recycledCount, nullptr);

        recycledIndices.resize(recycledCount);
        result = data.exportState(&nextIndex, &recycledCount, recycledIndices.data());
        recycledIndices.resize(recycledCount);
    } while (result.value != FOE_SUCCESS);

    try {
        // Next Free Index
        yaml_write_required("next_free_index", nextIndex, *pWriteNode);

        // Recycled Indices
        YAML::Node recycledNode;
        for (auto it : recycledIndices) {
            recycledNode.push_back(it);
        }
        (*pWriteNode)["recycled_indices"] = recycledNode;
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.whatStr()};
        }
    }

    if (!nodeName.empty()) {
        node[nodeName] = newNode;
    }
}