// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_ECS_YAML_ID_HPP
#define FOE_ECS_YAML_ID_HPP

#include <foe/ecs/group_translator.h>
#include <foe/ecs/id.h>
#include <foe/ecs/yaml/export.h>
#include <yaml-cpp/yaml.h>

/** @brief Reads the GroupValue and Index portions of an ID
 * @param nodeName The name of the sub-node to read the id from, or empty for the given node
 * @param node Yaml node to parse
 * @param groupTranslator If given, translates the read-in original GroupID and gets the translated
 * one
 * @param id Returns the final foeId value
 * @throws A descriptive exception on failure to parse.
 *
 * groupValue is from the 'group_id' node.
 * index is read from the 'index_id' node.
 */
FOE_ECS_YAML_EXPORT void yaml_read_id_required(std::string const &nodeName,
                                               YAML::Node const &node,
                                               foeEcsGroupTranslator groupTranslator,
                                               foeId &id);

/** @brief Reads the GroupValue and Index portions of an ID
 * @param nodeName The name of the sub-node to read the id from, or empty for the given node
 * @param node Yaml node to parse
 * @param groupTranslator If given, translates the read-in original GroupID and gets the translated
 * one
 * @param id Returns the final foeId value
 * @throws A descriptive exception on failure to parse.
 * @returns True if the ID was read in, false if the index_id node was missing
 *
 * groupValue is from the 'group_id' node.
 * index is read from the 'index_id' node.
 */
FOE_ECS_YAML_EXPORT bool yaml_read_id_optional(std::string const &nodeName,
                                               YAML::Node const &node,
                                               foeEcsGroupTranslator groupTranslator,
                                               foeId &id);

/** @brief Writes the given ID's GroupValue and Index to the Yaml node
 * @param id ID to be converted to Yaml
 * @param node Yaml node to be written to
 * @warning If the IdGroup is the 'persistent' group, then the group_id node isn't written
 *
 * The IdGroup is written to the 'group_id' node.
 * The IdIndex is written to the 'index_id' node.
 */
FOE_ECS_YAML_EXPORT void yaml_write_id(std::string const &nodeName, foeId id, YAML::Node &node);

#endif // FOE_ECS_YAML_ID_HPP