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

#include <foe/resource/yaml/image.hpp>

#include <foe/log.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include "image.hpp"

namespace {

bool yaml_read_image_definition_internal(std::string const &nodeName,
                                         YAML::Node const &node,
                                         foeIdGroupTranslator const *pTranslator,
                                         foeImageCreateInfo &createInfo) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    try {
        // Resources

        // Graphics Data

        // Other Data
        yaml_read_required("file", subNode, createInfo.fileName);
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException(nodeName + "::" + e.what());
        }
    }

    return true;
}

} // namespace

void yaml_read_image_definition(YAML::Node const &node,
                                foeIdGroupTranslator const *pTranslator,
                                foeResourceCreateInfoBase **ppCreateInfo) {
    foeImageCreateInfo ci;

    yaml_read_image_definition_internal("", node, pTranslator, ci);

    *ppCreateInfo = new foeImageCreateInfo(std::move(ci));
}