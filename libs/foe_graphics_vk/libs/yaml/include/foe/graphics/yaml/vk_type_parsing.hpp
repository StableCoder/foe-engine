/*
    Copyright (C) 2020 George Cave.

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

#ifndef FOE_GRAPHICS_YAML_VK_TYPE_PARSING_HPP
#define FOE_GRAPHICS_YAML_VK_TYPE_PARSING_HPP

#include <foe/graphics/yaml/export.h>
#include <yaml-cpp/yaml.h>

#include <string>

/** @brief Read a node and parse it into the given data type
 * @tparam VkType Vulkan type to parse/serialize
 * @param typeName String name of the Vulkan type
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param node Yaml node that should have the sub-node to operate on
 * @param data [out] Raw data being worked on
 * @return True on a successful read. The function returns an exception if the 'required' node
 * isn'VkType found instead.
 * @exception Throws foeYamlException if the required node is not found or there's an error
 * during parsing
 */
template <typename VkType>
FOE_GFX_YAML_EXPORT bool yaml_read_required_vk(std::string const &typeName,
                                               std::string const &nodeName,
                                               YAML::Node const &node,
                                               VkType &data);

/** @brief Read a node and parse it into the given data type
 * @tparam VkType Vulkan type to parse/serialize
 * @param typeName String name of the Vulkan type
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param node Yaml node that should have the sub-node to operate on
 * @param data [out] Object to read the data into. If false is returned, then the object is not
 * modified.
 * @return True on a successful read. False if the node isn'VkType found.
 * @exception Throws a foeYamlException if there's an error during parsing
 */
template <typename VkType>
FOE_GFX_YAML_EXPORT bool yaml_read_optional_vk(std::string const &typeName,
                                               std::string const &nodeName,
                                               YAML::Node const &node,
                                               VkType &data);

/** @brief Encodes the given data object to Yaml
 * @tparam VkType Vulkan type to parse/serialize
 * @param typeName String name of the Vulkan type
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param data Data to be encoded
 * @param node [out] Yaml node that should have the sub-node to operate on
 * @return True on a successful write. Returns an exception if there's an error when writing.
 * @exception Throws foeYamlException if there's an exception during encoding
 */
template <typename VkType>
FOE_GFX_YAML_EXPORT bool yaml_write_required_vk(std::string const &typeName,
                                                std::string const &nodeName,
                                                VkType const &data,
                                                YAML::Node &node);

/** @brief Encodes the given data object to Yaml optionally
 * @tparam VkType Vulkan type to parse/serialize
 * @param typeName String name of the Vulkan type
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param data Raw data being encoded
 * @param defaultData The default value of the data. If data matches this, then it is not written
 * out.
 * @param node [out] Yaml node that should have the sub-node to operate on
 * @return True on a successful write. False if nothing is written out.
 * @exception Throws foeYamlException if there's an exception during encoding
 */
template <typename VkType>
FOE_GFX_YAML_EXPORT bool yaml_write_optional_vk(std::string const &typeName,
                                                std::string const &nodeName,
                                                VkType const &data,
                                                VkType const &defaultData,
                                                YAML::Node &node);

#endif // FOE_GRAPHICS_YAML_VK_TYPE_PARSING_HPP