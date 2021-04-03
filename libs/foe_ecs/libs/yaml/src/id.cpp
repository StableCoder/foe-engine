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

#include <foe/ecs/groups.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

bool yaml_read_id(YAML::Node const &node,
                  // foeIdGroup targetGroup,
                  foeId &id,
                  foeEditorNameMap *pEditorNameMap) {
    try {
        { // ID
            foeIdGroup idGroup = 0;
            /*
            if (yaml_read_optional("group_id", node, idGroup)) {
                idGroup = foeEcsNormalizedToGroupID(idGroup);
            }
            */

            foeIdIndex idIndex;
            yaml_read_required("index_id", node, idIndex);

            id = idGroup | idIndex;
        }

        // Editor Name
        if (auto editorNameMap = node["editor_name"]; editorNameMap && pEditorNameMap != nullptr) {
            std::string editorName;
            yaml_read_required("", editorNameMap, editorName);

            if (!pEditorNameMap->add(id, editorName)) {
                return false;
            }
        }
    } catch (foeYamlException const &e) {
        throw e;
    }

    return true;
}

auto yaml_write_id(foeId id, foeEditorNameMap *pEditorNameMap) -> YAML::Node {
    YAML::Node writeNode;

    try {
        { // ID
            yaml_write_required("index_id", foeEcsGetIndexID(id), writeNode);

            // Write out the IdGroup if it is part of a dependency group.
            if (foeEcsGetGroupID(id) != foeEcsGroups::Persistent) {
                yaml_write_required("group_id", foeEcsGetGroupID(id), writeNode);
            }
        }

        // Editor Name
        if (pEditorNameMap != nullptr) {
            if (auto editorName = pEditorNameMap->find(id); !editorName.empty()) {
                yaml_write_required("editor_name", editorName, writeNode);
            }
        }
    } catch (foeYamlException const &e) {
        throw e;
    }

    return writeNode;
}