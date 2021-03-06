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

#include <foe/graphics/yaml/builtin_descriptor_sets.hpp>

#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>

#include <limits>

void yaml_write_builtin_descriptor_set_layouts(std::string const &nodeName,
                                               foeBuiltinDescriptorSetLayoutFlags const &data,
                                               YAML::Node &node) {
    YAML::Node writeNode;

    try {
        for (size_t i = 0;
             i < std::numeric_limits<foeBuiltinDescriptorSetLayoutFlags>::digits && data != 0;
             ++i) {
            foeBuiltinDescriptorSetLayoutFlagBits setFlag =
                static_cast<foeBuiltinDescriptorSetLayoutFlagBits>(1 << i);

            if ((data & setFlag) != 0) {
                writeNode.push_back(to_string(setFlag));
            }
        }
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.whatStr()};
        }
    }

    if (nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}

bool yaml_read_builtin_descriptor_set_layouts(std::string const &nodeName,
                                              YAML::Node const &node,
                                              foeBuiltinDescriptorSetLayoutFlags &data) {
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if (!subNode) {
        return false;
    }

    foeBuiltinDescriptorSetLayoutFlags tempData = 0;
    try {
        tempData = 0;
        if (auto builtinsNode = subNode; builtinsNode) {
            for (auto it = builtinsNode.begin(); it != builtinsNode.end(); ++it) {
                std::string builtinName;
                yaml_read_required("", *it, builtinName);

                tempData |= to_builtin_set_layout(builtinName);
            }
        }
    } catch (foeYamlException const &e) {
        if (nodeName.empty()) {
            throw e;
        } else {
            throw foeYamlException{nodeName + "::" + e.whatStr()};
        }
    }

    data = tempData;
    return true;
}