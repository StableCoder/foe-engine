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

auto yaml_read_id(YAML::Node const &node,
                  // foeIdGroup targetGroup,
                  foeId &id) -> foeId {
    try {
        // Group
        foeIdGroup idGroup = 0;
        /*
        if (yaml_read_optional("group_id", node, idGroup)) {
            idGroup = foeEcsNormalizedToGroupID(idGroup);
        }
        */

        // Index
        foeIdIndex idIndex;
        yaml_read_required("index_id", node, idIndex);

        return idGroup | idIndex;
    } catch (foeYamlException const &e) {
        throw e;
    }
}

void yaml_write_id(foeId id, YAML::Node &node) {
    try {
        // Index
        yaml_write_required("index_id", foeEcsGetIndexID(id), node);

        // Group, if not part of the persistent group
        if (foeEcsGetGroupID(id) != foeEcsGroups::Persistent) {
            yaml_write_required("group_id", foeEcsGetGroupID(id), node);
        }
    } catch (foeYamlException const &e) {
        throw e;
    }
}