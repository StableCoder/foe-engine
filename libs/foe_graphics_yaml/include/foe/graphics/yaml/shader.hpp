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

FOE_GFX_YAML_EXPORT bool yaml_read_shader_declaration(std::string const &nodeName,
                                                      YAML::Node const &node,
                                                      foeShaderPool *pShaderPool,
                                                      foeShader **pShader);

FOE_GFX_YAML_EXPORT bool yaml_write_shader_declaration(std::string const &nodeName,
                                                       foeShader const *pShader,
                                                       YAML::Node &node);

FOE_GFX_YAML_EXPORT bool yaml_read_shader_definition(
    std::string const &nodeName,
    YAML::Node const &node,
    foeDescriptorSetLayoutPool *pDescriptorSetLayoutPool,
    foeShader *pShader);

FOE_GFX_YAML_EXPORT bool yaml_write_shader_definition(
    std::string const &nodeName,
    foeBuiltinDescriptorSets const *pBuiltinDescriptorSets,
    foeDescriptorSetLayoutPool const *pDescriptorSetLayoutPool,
    foeShader const *pShader,
    YAML::Node &node);

#endif // FOE_GRAPHICS_YAML_SHADER_HPP