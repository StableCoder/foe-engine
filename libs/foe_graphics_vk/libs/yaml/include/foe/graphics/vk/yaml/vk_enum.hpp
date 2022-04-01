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

#ifndef FOE_GRAPHICS_VK_YAML_VK_ENUM_HPP
#define FOE_GRAPHICS_VK_YAML_VK_ENUM_HPP

#include <foe/graphics/vk/yaml/export.h>
#include <vulkan/vulkan.h>
#include <yaml-cpp/yaml.h>

#include <string>

/** @brief Read a node and parse it into the given data type (32-bit)
 * @param typeName String name of the Vulkan type
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param node Yaml node that should have the sub-node to operate on
 * @param data [out] Raw data being worked on
 * @exception Throws foeYamlException if the required node is not found or there's an error
 * during parsing
 */
FOE_GFX_VK_YAML_EXPORT void yaml_read_required_VkEnum32(std::string const &typeName,
                                                        std::string const &nodeName,
                                                        YAML::Node const &node,
                                                        VkFlags &data);

/** @brief Read a node and parse it into the given data type (32-bit)
 * @param typeName String name of the Vulkan type
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param node Yaml node that should have the sub-node to operate on
 * @param data [out] Object to read the data into. If false is returned, then the object is not
 * modified.
 * @return True on a successful read. False if the node isn'VkType found.
 * @exception Throws a foeYamlException if there's an error during parsing
 */
FOE_GFX_VK_YAML_EXPORT bool yaml_read_optional_VkEnum32(std::string const &typeName,
                                                        std::string const &nodeName,
                                                        YAML::Node const &node,
                                                        VkFlags &data);

/** @brief Encodes the given data object to Yaml (32-bit)
 * @param typeName String name of the Vulkan type
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param data Data to be encoded
 * @param node [out] Yaml node that should have the sub-node to operate on
 * @exception Throws foeYamlException if there's an exception during encoding
 */
FOE_GFX_VK_YAML_EXPORT void yaml_write_required_VkEnum32(std::string const &typeName,
                                                         std::string const &nodeName,
                                                         VkFlags const &data,
                                                         YAML::Node &node);

/** @brief Encodes the given data object to Yaml optionally (32-bit)
 * @param typeName String name of the Vulkan type
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param defaultData The default value of the data. If data matches this, then it is not written
 * out.
 * @param data Raw data being encoded
 * @param node [out] Yaml node that should have the sub-node to operate on
 * @return True on a successful write. False if nothing is written out.
 * @exception Throws foeYamlException if there's an exception during encoding
 */
FOE_GFX_VK_YAML_EXPORT bool yaml_write_optional_VkEnum32(std::string const &typeName,
                                                         std::string const &nodeName,
                                                         VkFlags const &defaultData,
                                                         VkFlags const &data,
                                                         YAML::Node &node);

/** @brief Read a node and parse it into the given data type (64-bit)
 * @param typeName String name of the Vulkan type
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param node Yaml node that should have the sub-node to operate on
 * @param data [out] Raw data being worked on
 * @exception Throws foeYamlException if the required node is not found or there's an error
 * during parsing
 */
FOE_GFX_VK_YAML_EXPORT void yaml_read_required_VkEnum64(std::string const &typeName,
                                                        std::string const &nodeName,
                                                        YAML::Node const &node,
                                                        VkFlags64 &data);

/** @brief Read a node and parse it into the given data type (64-bit)
 * @param typeName String name of the Vulkan type
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param node Yaml node that should have the sub-node to operate on
 * @param data [out] Object to read the data into. If false is returned, then the object is not
 * modified.
 * @return True on a successful read. False if the node isn'VkType found.
 * @exception Throws a foeYamlException if there's an error during parsing
 */
FOE_GFX_VK_YAML_EXPORT bool yaml_read_optional_VkEnum64(std::string const &typeName,
                                                        std::string const &nodeName,
                                                        YAML::Node const &node,
                                                        VkFlags64 &data);

/** @brief Encodes the given data object to Yaml (64-bit)
 * @param typeName String name of the Vulkan type
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param data Data to be encoded
 * @param node [out] Yaml node that should have the sub-node to operate on
 * @exception Throws foeYamlException if there's an exception during encoding
 */
FOE_GFX_VK_YAML_EXPORT void yaml_write_required_VkEnum64(std::string const &typeName,
                                                         std::string const &nodeName,
                                                         VkFlags64 const &data,
                                                         YAML::Node &node);

/** @brief Encodes the given data object to Yaml optionally
 * @tparam VkType Vulkan type to parse/serialize
 * @param typeName String name of the Vulkan type
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param defaultData The default value of the data. If data matches this, then it is not written
 * out.
 * @param data Raw data being encoded
 * @param node [out] Yaml node that should have the sub-node to operate on
 * @return True on a successful write. False if nothing is written out.
 * @exception Throws foeYamlException if there's an exception during encoding
 */
FOE_GFX_VK_YAML_EXPORT bool yaml_write_optional_VkEnum64(std::string const &typeName,
                                                         std::string const &nodeName,
                                                         VkFlags64 const &defaultData,
                                                         VkFlags64 const &data,
                                                         YAML::Node &node);

/** @brief Read a node and parse it into the given data type (template)
 * @tparam T Vulkan type to parse/serialize
 * @param typeName String name of the Vulkan type
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param node Yaml node that should have the sub-node to operate on
 * @param data [out] Raw data being worked on
 * @exception Throws foeYamlException if the required node is not found or there's an error
 * during parsing
 */
template <typename T>
void yaml_read_required_VkEnum(std::string const &typeName,
                               std::string const &nodeName,
                               YAML::Node const &node,
                               T &data) {
    static_assert(sizeof(T) == 4 || sizeof(T) == 8,
                  "yaml_read_required_VkEnum only supports 32 and 64-bit types currently.");

    if constexpr (sizeof(T) == 4) {
        yaml_read_required_VkEnum32(typeName, nodeName, node, reinterpret_cast<VkFlags &>(data));
    } else {
        yaml_read_required_VkEnum64(typeName, nodeName, node, reinterpret_cast<VkFlags64 &>(data));
    }
}

/** @brief Read a node and parse it into the given data type (template)
 * @tparam T Vulkan type to parse/serialize
 * @param typeName String name of the Vulkan type
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param node Yaml node that should have the sub-node to operate on
 * @param data [out] Object to read the data into. If false is returned, then the object is not
 * modified.
 * @return True on a successful read. False if the node isn'VkType found.
 * @exception Throws a foeYamlException if there's an error during parsing
 */
template <typename T>
bool yaml_read_optional_VkEnum(std::string const &typeName,
                               std::string const &nodeName,
                               YAML::Node const &node,
                               T &data) {
    static_assert(sizeof(T) == 4 || sizeof(T) == 8,
                  "yaml_read_optional_VkEnum only supports 32 and 64-bit types currently.");

    if (sizeof(T) == 4) {
        return yaml_read_optional_VkEnum32(typeName, nodeName, node,
                                           reinterpret_cast<VkFlags &>(data));
    } else {
        return yaml_read_optional_VkEnum64(typeName, nodeName, node,
                                           reinterpret_cast<VkFlags64 &>(data));
    }
}

/** @brief Encodes the given data object to Yaml (template)
 * @tparam T Vulkan type to parse/serialize
 * @param typeName String name of the Vulkan type
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param data Data to be encoded
 * @param node [out] Yaml node that should have the sub-node to operate on
 * @exception Throws foeYamlException if there's an exception during encoding
 */
template <typename T>
void yaml_write_required_VkEnum(std::string const &typeName,
                                std::string const &nodeName,
                                T const &data,
                                YAML::Node &node) {
    static_assert(sizeof(T) == 4 || sizeof(T) == 8,
                  "yaml_write_required_VkEnum only supports 32 and 64-bit types currently.");

    if (sizeof(T) == 4) {
        yaml_write_required_VkEnum32(typeName, nodeName, static_cast<VkFlags const &>(data), node);
    } else {
        yaml_write_required_VkEnum64(typeName, nodeName, static_cast<VkFlags64 const &>(data),
                                     node);
    }
}

/** @brief Encodes the given data object to Yaml optionally (template)
 * @tparam T Vulkan type to parse/serialize
 * @param typeName String name of the Vulkan type
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param defaultData The default value of the data. If data matches this, then it is not written
 * out.
 * @param data Raw data being encoded
 * @param node [out] Yaml node that should have the sub-node to operate on
 * @return True on a successful write. False if nothing is written out.
 * @exception Throws foeYamlException if there's an exception during encoding
 */
template <typename T>
bool yaml_write_optional_VkEnum(std::string const &typeName,
                                std::string const &nodeName,
                                T const &defaultData,
                                T const &data,
                                YAML::Node &node) {
    static_assert(sizeof(T) == 4 || sizeof(T) == 8,
                  "yaml_write_optional_VkEnum only supports 32 and 64-bit types currently.");

    if (sizeof(T) == 4) {
        return yaml_write_optional_VkEnum32(typeName, nodeName,
                                            static_cast<VkFlags const &>(defaultData),
                                            static_cast<VkFlags const &>(data), node);
    } else {
        return yaml_write_optional_VkEnum64(typeName, nodeName,
                                            static_cast<VkFlags64 const &>(defaultData),
                                            static_cast<VkFlags64 const &>(data), node);
    }
}

#endif // FOE_GRAPHICS_VK_YAML_VK_ENUM_HPP