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

#include "image.hpp"

bool yaml_read_image_definition(YAML::Node const &node,
                                foeIdGroupTranslator const *pTranslator,
                                foeImageCreateInfo &createInfo) {
    try {
        yaml_read_image_definition("", node, pTranslator, createInfo.fileName);
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to import foeImage definition: {}", e.what());
        return false;
    }

    return true;
}

void yaml_read_image_definition2(YAML::Node const &node,
                                 foeIdGroupTranslator const *pTranslator,
                                 foeResourceCreateInfoBase **ppCreateInfo) {
    foeImageCreateInfo ci;

    yaml_read_image_definition(node, pTranslator, ci);

    *ppCreateInfo = new foeImageCreateInfo(std::move(ci));
}