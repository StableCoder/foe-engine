// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/ecs/yaml/id.hpp>
#include <foe/imex/yaml/importer.hpp>
#include <foe/simulation/simulation.hpp>
#include <foe/yaml/exception.hpp>

#include "test_common.hpp"

namespace {

// Component
bool importIdComponent(YAML::Node const &node,
                       foeEcsGroupTranslator groupTranslator,
                       foeEntityID entity,
                       foeSimulation const *pSimulation) {
    if (auto dataNode = node[cNodeKey]; dataNode) {
        try {
            foeId readID;

            yaml_read_foeEntityID("", dataNode, groupTranslator, readID);

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