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

#ifndef FOE_GRAPHICS_YAML_BUILTIN_DESCRIPTOR_SETS_HPP
#define FOE_GRAPHICS_YAML_BUILTIN_DESCRIPTOR_SETS_HPP

#include <foe/graphics/builtin_descriptor_sets.hpp>
#include <foe/graphics/yaml/export.h>
#include <yaml-cpp/yaml.h>

#include <string>

/**
 * @brief Read a node and parse it into the given data type
 * @param nodeName Name of the Yaml sub-node to operate on (blank to operator on given node)
 * @param node Yaml node that should have the sub-node to operate on
 * @param data [out] Raw data being worked on
 * @return True if the node was found/parsed, false if the node could not be found.
 * @exception Throws foeYamlException on any parsing issue or any exception is caught.
 */
FOE_GFX_YAML_EXPORT bool yaml_read_builtin_descriptor_set_layouts(
    std::string const &nodeName, YAML::Node const &node, foeBuiltinDescriptorSetLayoutFlags &data);

/** @brief Encodes the given data object to Yaml
 * @param nodeName Name of the Yaml sub-node to operate on (blank to operator on given node)
 * @param data Data to be encoded
 * @param node [out] Yaml node to operate on
 * @exception Throws foeYamlException if there's any exception during writing
 */
FOE_GFX_YAML_EXPORT void yaml_write_builtin_descriptor_set_layouts(
    std::string const &nodeName, foeBuiltinDescriptorSetLayoutFlags const &data, YAML::Node &node);

#endif // FOE_GRAPHICS_YAML_BUILTIN_DESCRIPTOR_SETS_HPP