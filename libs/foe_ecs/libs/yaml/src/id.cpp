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
                           foeId &id) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        throw foeYamlException(nodeName + " - Required node to parse foeId not found");
    }

    try {
        if (!yaml_read_id_optional("", node, pTranslator, id)) {
            throw foeYamlException(nodeName +
                                   "::index_id - Coudl not find required node to parse foeId");
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
                           foeId &id) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        // Group
        foeIdGroup group = foeIdPersistentGroup;
        foeIdGroupValue groupValue;
        if (yaml_read_optional("group_id", node, groupValue)) {
            // If we were given a translator, use it otherwise the value is just converted
            if (pTranslator != nullptr) {
                group = foeIdTranslateGroupValue(pTranslator, groupValue);
            } else {
                group = foeIdValueToGroup(groupValue);
            }
        }

        // Index
        foeIdIndex index;
        if (!yaml_read_optional("index_id", node, index))
            return false;

        id = foeIdCreate(group, index);
    } catch (foeYamlException const &e) {
        throw e;
    }

    return true;
}

void yaml_write_id(foeId id, YAML::Node &node) {
    try {
        // Index
        yaml_write_required("index_id", foeIdGetIndex(id), node);

        // Group, if not part of the persistent group
        if (foeIdGetGroup(id) != foeIdPersistentGroup) {
            yaml_write_required("group_id", foeIdGroupToValue(id), node);
        }
    } catch (foeYamlException const &e) {
        throw e;
    }
}