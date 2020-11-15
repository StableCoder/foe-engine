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

#ifndef FOE_GRAPHICS_YAML_SHADER_HPP
#define FOE_GRAPHICS_YAML_SHADER_HPP

#include <foe/graphics/yaml/export.h>
#include <yaml-cpp/yaml.h>

#include <string>

class foeShaderPool;
class foeShader;
class foeBuiltinDescriptorSets;
class foeDescriptorSetLayoutPool;

/** @brief Parses the type declaration, and uses that to find/generate one
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param node Yaml node that should have the sub-node to operate on
 * @param pShaderPool Pool used to search for/generate items to be returned
 * @param pShader [out] Return data if parsing is successful
 * @return True if parsing was successful, false otherwise.
 * @exception Throws foeYamlException if the specified node was not found or another error
 */
FOE_GFX_YAML_EXPORT bool yaml_read_shader_declaration(std::string const &nodeName,
                                                      YAML::Node const &node,
                                                      foeShaderPool *pShaderPool,
                                                      foeShader **pShader);

/** @brief Serializes a declaration of the given type
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param pShader Data to be serialized
 * @param node [out] Yaml node that the data is being written to, or to a subnode of
 * @return True if the serialization was successful, false otherwise
 * @exception Throws foeYamlException with a description if there's an failure during serialization
 */
FOE_GFX_YAML_EXPORT bool yaml_write_shader_declaration(std::string const &nodeName,
                                                       foeShader const *pShader,
                                                       YAML::Node &node);

/** @brief Parses the type declaration, and uses that to find/generate one
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param node Yaml node that should have the sub-node to operate on
 * @param pDescriptorSetLayoutPool Pool used to search for/generate items to be returned
 * @param pShader Item the data is being defined for
 * @return True if parsing was successful, false otherwise.
 * @exception Throws foeYamlException if the specified node was not found or another error
 */
FOE_GFX_YAML_EXPORT bool yaml_read_shader_definition(
    std::string const &nodeName,
    YAML::Node const &node,
    foeDescriptorSetLayoutPool *pDescriptorSetLayoutPool,
    foeShader *pShader);

/** @brief Serializes a declaration of the given type
 * @param nodeName Name of the Yaml sub-node to operate on
 * @param pMaterial Data to be serialized
 * @param node [out] Yaml node that the data is being written to, or to a subnode of
 * @return True if the serialization was successful, false otherwise
 * @exception Throws foeYamlException with a description if there's an failure during serialization
 */
FOE_GFX_YAML_EXPORT bool yaml_write_shader_definition(
    std::string const &nodeName,
    foeBuiltinDescriptorSets const *pBuiltinDescriptorSets,
    foeDescriptorSetLayoutPool const *pDescriptorSetLayoutPool,
    foeShader const *pShader,
    YAML::Node &node);

#endif // FOE_GRAPHICS_YAML_SHADER_HPP