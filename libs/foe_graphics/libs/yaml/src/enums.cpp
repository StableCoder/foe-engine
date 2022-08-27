// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/yaml/enums.hpp>

#include <foe/yaml/exception.hpp>
#include <foe/yaml/pod.hpp>

#include <limits>

bool yaml_read_foeBuiltinDescriptorSetLayoutFlags(std::string const &nodeName,
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
                yaml_read_string("", *it, builtinName);

                tempData |= string_to_builtin_set_layout(builtinName.c_str());
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

void yaml_write_foeBuiltinDescriptorSetLayoutFlags(std::string const &nodeName,
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
                char const *pStr = builtin_set_layout_to_string(setFlag);
                if (pStr != NULL)
                    writeNode.push_back(pStr);
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