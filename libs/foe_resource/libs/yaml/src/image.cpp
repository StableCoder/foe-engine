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

#include "image.hpp"

#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

bool yaml_write_image_declaration(std::string const &nodeName,
                                  foeImage const *pImage,
                                  YAML::Node &node) {
    try {
        if (nodeName.empty()) {
            node = pImage->getID();
        } else {
            node[nodeName] = pImage->getID();
        }
    } catch (...) {
        throw foeYamlException(nodeName + " - Failed to serialize 'foeShader' declaration");
    }

    return true;
}
