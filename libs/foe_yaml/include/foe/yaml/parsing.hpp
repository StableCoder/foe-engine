// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_YAML_PARSING_HPP
#define FOE_YAML_PARSING_HPP

#include <yaml-cpp/yaml.h>

#include <string>

/** @brief Read a node and parse it into the given data type
 * @param nodeName Name of the Yaml sub-node to operate on. If empty, uses the given node directly.
 * @param node Yaml node that should have the sub-node to operate on
 * @param data [out] Raw data being worked on
 * @exception Throws foeYamlException if the required node is not found or there's an error
 * during parsing
 */
template <typename T>
void yaml_read_required(std::string const &nodeName, YAML::Node const &node, T &data);

/** @brief Read a node and parse it into the given data type
 * @param nodeName Name of the Yaml sub-node to operate on. If empty, uses the given node directly.
 * @param node Yaml node that should have the sub-node to operate on
 * @param data [out] Object to read the data into. If false is returned, then the object is not
 * modified.
 * @return True on a successful read. False if the node isn't found.
 * @exception Throws a foeYamlException if there's an error during parsing
 */
template <typename T>
bool yaml_read_optional(std::string const &nodeName, YAML::Node const &node, T &data);

/** @brief Encodes the given data object to Yaml
 * @param nodeName Name of the Yaml sub-node to operate on. If empty, uses the given node directly.
 * @param data Data to be encoded
 * @param node [out] Yaml node that should have the sub-node to operate on
 * @exception Throws foeYamlException if there's an exception during encoding
 */
template <typename T>
void yaml_write_required(std::string const &nodeName, T const &data, YAML::Node &node);

/** @brief Encodes the given data object to Yaml optionally
 * @param nodeName Name of the Yaml sub-node to operate on. If empty, uses the given node directly.
 * @param defaultData The default value of the data. If data matches this, then it is not written
 * out.
 * @param data Raw data being encoded
 * @param node [out] Yaml node that should have the sub-node to operate on
 * @return True on a successful write. False if nothing is written out.
 * @exception Throws foeYamlException if there's an exception during encoding
 * @warning The specified node is overwritten with the provided data.
 */
template <typename T>
bool yaml_write_optional(std::string const &nodeName,
                         T const &defaultData,
                         T const &data,
                         YAML::Node &node);

#endif // FOE_YAML_PARSING_HPP