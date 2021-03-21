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

bool import_yaml_image_definition(std::string_view imageName, std::string &fileName) {
    // Open the YAML file
    YAML::Node rootNode;
    try {
        rootNode = YAML::LoadFile(std::string{imageName} + ".yml");
    } catch (YAML::ParserException &e) {
        FOE_LOG(General, Fatal, "Failed to load Yaml file: {}", e.what());
        return false;
    }

    try {
        yaml_read_image_definition("", rootNode, fileName);
    } catch (foeYamlException const &e) {
        FOE_LOG(General, Error, "Failed to import foeImage definition: {}", e.what());
        return false;
    }

    return true;
}