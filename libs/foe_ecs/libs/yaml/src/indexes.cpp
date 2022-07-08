// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/ecs/yaml/indexes.hpp>

#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

void yaml_read_indexes(std::string const &nodeName, YAML::Node const &node, foeEcsIndexes indexes) {
    YAML::Node const &readNode = (nodeName.empty()) ? node : node[nodeName];
    if (!readNode) {
        throw foeYamlException(nodeName + " - Required node to parse not found");
    }

    foeIdIndex nextNewIndex;
    std::vector<foeIdIndex> recycledIndices;

    // Next Free Index
    try {
        yaml_read_required("next_free_index", readNode, nextNewIndex);
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

    foeEcsImportIndexes(indexes, nextNewIndex, recycledIndices.size(), recycledIndices.data());
}

void yaml_write_indexes(std::string const &nodeName, foeEcsIndexes indexes, YAML::Node &node) {
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
    foeIdIndex nextNewIndex;
    std::vector<foeIdIndex> recycledIndices;

    do {
        uint32_t recycledCount;
        foeEcsExportIndexes(indexes, nullptr, &recycledCount, nullptr);

        recycledIndices.resize(recycledCount);
        result =
            foeEcsExportIndexes(indexes, &nextNewIndex, &recycledCount, recycledIndices.data());
        recycledIndices.resize(recycledCount);
    } while (result.value != FOE_SUCCESS);

    try {
        // Next Free Index
        yaml_write_required("next_free_index", nextNewIndex, *pWriteNode);

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