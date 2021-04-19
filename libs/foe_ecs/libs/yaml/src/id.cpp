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

#include <foe/ecs/yaml/id.hpp>

#include <foe/ecs/group_translator.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

void yaml_read_id_required(std::string const &nodeName,
                           YAML::Node const &node,
                           foeIdGroupTranslator const *pTranslator,
                           foeIdType idType,
                           foeId &id) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        throw foeYamlException(nodeName + " - Required node to parse foeId not found");
    }

    try {
        if (!yaml_read_id_optional("", subNode, pTranslator, idType, id)) {
            throw foeYamlException("index_id - Could not find required node to parse foeId");
        }
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException(nodeName + "::" + e.whatStr());
        }
    }
}

bool yaml_read_id_optional(std::string const &nodeName,
                           YAML::Node const &node,
                           foeIdGroupTranslator const *pTranslator,
                           foeIdType idType,
                           foeId &id) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        // Group
        foeIdGroup group = foeIdPersistentGroup;
        foeIdGroupValue groupValue;
        if (yaml_read_optional("group_id", subNode, groupValue)) {
            // If we were given a translator, use it otherwise the value is just converted
            if (pTranslator != nullptr) {
                group = foeIdTranslateGroupValue(pTranslator, groupValue);
            } else {
                group = foeIdValueToGroup(groupValue);
            }
        }

        // Index
        foeIdIndex index;
        if (!yaml_read_optional("index_id", subNode, index))
            return false;

        id = foeIdCreateType(group, idType, index);
    } catch (foeYamlException const &e) {
        throw e;
    }

    return true;
}

void yaml_write_id(std::string const &nodeName, foeId data, YAML::Node &node) {
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

    try {
        yaml_write_optional("group_id", foeIdGroupToValue(foeIdPersistentGroup),
                            foeIdGroupToValue(foeIdGetGroup(data)), *pWriteNode);

        yaml_write_required("index_id", foeIdIndexToValue(data), *pWriteNode);
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