// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/ecs/yaml/id.hpp>

#include <foe/ecs/group_translator.h>
#include <foe/ecs/id_to_string.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

void yaml_read_id_required(std::string const &nodeName,
                           YAML::Node const &node,
                           foeEcsGroupTranslator groupTranslator,
                           foeId &id) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        throw foeYamlException(nodeName + " - Required node to parse foeId not found");
    }

    try {
        if (!yaml_read_id_optional("", subNode, groupTranslator, id)) {
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
                           foeEcsGroupTranslator groupTranslator,
                           foeId &id) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        // Group
        foeIdGroup group;
        foeIdGroupValue groupValue = foeIdPersistentGroupValue;
        yaml_read_optional("group_id", subNode, groupValue);
        if (groupTranslator != FOE_NULL_HANDLE) {
            foeResult result =
                foeEcsGetTranslatedGroup(groupTranslator, foeIdValueToGroup(groupValue), &group);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                throw foeYamlException("group_id - Was given groupValue of '" +
                                       std::to_string(groupValue) +
                                       "' for which no translation exists - " + buffer);
            }
        } else {
            group = foeIdValueToGroup(groupValue);
        }

        // Index
        foeIdIndex index;
        if (!yaml_read_optional("index_id", subNode, index))
            return false;

        id = foeIdCreate(group, index);
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
        yaml_write_optional("group_id", foeIdGroupToString(foeIdPersistentGroup),
                            foeIdGroupToString(foeIdGetGroup(data)), *pWriteNode);

        yaml_write_required("index_id", foeIdIndexToString(foeIdIndexToValue(data)), *pWriteNode);
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