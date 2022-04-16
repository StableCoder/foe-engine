/*
    Copyright (C) 2022 George Cave.

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
#include <foe/imex/yaml/importer.hpp>
#include <foe/simulation/simulation.hpp>
#include <foe/yaml/exception.hpp>

#include "test_common.hpp"

namespace {

// Component
bool importIdComponent(YAML::Node const &node,
                       foeIdGroupTranslator const *pGroupTranslator,
                       foeEntityID entity,
                       foeSimulation const *pSimulation) {
    if (auto dataNode = node[cNodeKey]; dataNode) {
        try {
            foeId readID;

            yaml_read_id_required("", dataNode, pGroupTranslator, readID);

            return true;
        } catch (foeYamlException const &e) {
            throw foeYamlException{std::string{cNodeKey} + "::" + e.whatStr()};
        }
    }

    return false;
}

} // namespace

bool registerTestImporterContent() {
    return foeImexYamlRegisterComponentFn(cNodeKey, importIdComponent);
}

void deregisterTestImporterContent() {
    foeImexYamlDeregisterComponentFn(cNodeKey, importIdComponent);
}