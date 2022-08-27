// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/ecs/yaml/id.hpp>

#include <foe/ecs/group_translator.h>
#include <foe/ecs/id_to_string.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/pod.hpp>

namespace {

bool yaml_read_id(std::string const &nodeName,
                  YAML::Node const &node,
                  foeEcsGroupTranslator groupTranslator,
                  foeId &id) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        // Group (Optional)
        foeIdGroup group;
        foeIdGroupValue groupValue = foeIdPersistentGroupValue;
        yaml_read_uint32_t("group_id", subNode, groupValue);
        if (groupTranslator != FOE_NULL_HANDLE) {
            foeResultSet result =
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
        if (!yaml_read_uint32_t("index_id", subNode, index)) {
            if (nodeName.empty()) {
                throw foeYamlException{
                    "index_id - Could not find required node to parse foe Index ID"};
            } else {
                throw foeYamlException{
                    nodeName + "::index_id - Could not find required node to parse foe Index ID"};
            }
        }

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
        if (foeIdGetGroup(data) != foeIdPersistentGroup)
            yaml_write_string("group_id", foeIdGroupToString(foeIdGetGroup(data)), *pWriteNode);

        yaml_write_string("index_id", foeIdIndexToString(foeIdIndexToValue(data)), *pWriteNode);
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

} // namespace

bool yaml_read_foeResourceID(std::string const &nodeName,
                             YAML::Node const &node,
                             foeEcsGroupTranslator groupTranslator,
                             foeResourceID &id) {
    return yaml_read_id(nodeName, node, groupTranslator, id);
}

bool yaml_read_foeEntityID(std::string const &nodeName,
                           YAML::Node const &node,
                           foeEcsGroupTranslator groupTranslator,
                           foeId &id) {
    return yaml_read_id(nodeName, node, groupTranslator, id);
}

void yaml_write_foeResourceID(std::string const &nodeName, foeResourceID id, YAML::Node &node) {
    yaml_write_id(nodeName, id, node);
}

void yaml_write_foeEntityID(std::string const &nodeName, foeId id, YAML::Node &node) {
    yaml_write_id(nodeName, id, node);
}