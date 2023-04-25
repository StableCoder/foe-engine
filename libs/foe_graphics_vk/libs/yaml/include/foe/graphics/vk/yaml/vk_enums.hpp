// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_VK_YAML_VK_ENUMS_HPP
#define FOE_GRAPHICS_VK_YAML_VK_ENUMS_HPP

#include <foe/graphics/vk/yaml/export.h>
#include <vulkan/vulkan.h>
#include <yaml-cpp/yaml.h>

#include <string>

/** @brief Read a node and parse it into the given data type (32-bit)
 * @param typeName String name of the Vulkan type
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param node Yaml node that should have the sub-node to operate on
 * @param data [out] Raw data being worked on
 * @return True if the node existed and was read. False if the node did not exist to be read.
 * @exception Throws foeYamlException if the required node is not found or there's an error
 * during parsing
 */
FOE_GFX_VK_YAML_EXPORT
bool yaml_read_VkEnum32(std::string const &typeName,
                        std::string const &nodeName,
                        YAML::Node const &node,
                        uint32_t &data);

/** @brief Encodes the given data object to Yaml (32-bit)
 * @param typeName String name of the Vulkan type
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param data Data to be encoded
 * @param node [out] Yaml node that should have the sub-node to operate on
 * @exception Throws foeYamlException if there's an exception during encoding
 */
FOE_GFX_VK_YAML_EXPORT
void yaml_write_VkEnum32(std::string const &typeName,
                         std::string const &nodeName,
                         uint32_t const &data,
                         YAML::Node &node);

/** @brief Read a node and parse it into the given data type (64-bit)
 * @param typeName String name of the Vulkan type
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param node Yaml node that should have the sub-node to operate on
 * @param data [out] Raw data being worked on
 * @return True if the node existed and was read. False if the node did not exist to be read.
 * @exception Throws foeYamlException if the required node is not found or there's an error
 * during parsing
 */
FOE_GFX_VK_YAML_EXPORT
bool yaml_read_VkEnum64(std::string const &typeName,
                        std::string const &nodeName,
                        YAML::Node const &node,
                        uint64_t &data);

/** @brief Encodes the given data object to Yaml (64-bit)
 * @param typeName String name of the Vulkan type
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param data Data to be encoded
 * @param node [out] Yaml node that should have the sub-node to operate on
 * @exception Throws foeYamlException if there's an exception during encoding
 */
FOE_GFX_VK_YAML_EXPORT
void yaml_write_VkEnum64(std::string const &typeName,
                         std::string const &nodeName,
                         uint64_t const &data,
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
bool yaml_read_VkEnum(std::string const &typeName,
                      std::string const &nodeName,
                      YAML::Node const &node,
                      T &data) {
    static_assert(sizeof(T) == 4 || sizeof(T) == 8,
                  "yaml_read_required_VkEnum only supports 32 and 64-bit types currently.");

    if constexpr (sizeof(T) == 4) {
        return yaml_read_VkEnum32(typeName, nodeName, node, reinterpret_cast<uint32_t &>(data));
    } else {
        return yaml_read_VkEnum64(typeName, nodeName, node, reinterpret_cast<uint64_t &>(data));
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
void yaml_write_VkEnum(std::string const &typeName,
                       std::string const &nodeName,
                       T const &data,
                       YAML::Node &node) {
    static_assert(sizeof(T) == 4 || sizeof(T) == 8,
                  "yaml_write_required_VkEnum only supports 32 and 64-bit types currently.");

    if (sizeof(T) == 4) {
        yaml_write_VkEnum32(typeName, nodeName, static_cast<uint32_t const &>(data), node);
    } else {
        yaml_write_VkEnum64(typeName, nodeName, static_cast<uint64_t const &>(data), node);
    }
}

#endif // FOE_GRAPHICS_VK_YAML_VK_ENUMS_HPP