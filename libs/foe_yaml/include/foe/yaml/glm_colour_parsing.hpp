// Copyright (C) 2021 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_YAML_GLM_COLOUR_PARSING_HPP
#define FOE_YAML_GLM_COLOUR_PARSING_HPP

#include <yaml-cpp/yaml.h>

#include <string>

/** @brief Encodes the given data object to Yaml
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param data Data to be encoded
 * @param node [out] Yaml node that should have the sub-node to operate on
 * @return True on a successful write. Returns an exception if there's an error when writing.
 * @exception Throws foeYamlException if there's an exception during encoding
 */
template <typename T>
void yaml_write_required_glm_colour(std::string const &nodeName, T const &data, YAML::Node &node);

/** @brief Encodes the given data object to Yaml optionally
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param defaultData The default value of the data. If data matches this, then it is not written
 * out.
 * @param data Raw data being encoded
 * @param node [out] Yaml node that should have the sub-node to operate on
 * @return True on a successful write. False if nothing is written out.
 * @exception Throws foeYamlException if there's an exception during encoding
 */
template <typename T>
bool yaml_write_optional_glm_colour(std::string const &nodeName,
                                    T const &defaultData,
                                    T const &data,
                                    YAML::Node &node);

#endif // FOE_YAML_GLM_COLOUR_PARSING_HPP