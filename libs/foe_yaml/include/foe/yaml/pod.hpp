// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_YAML_POD_HPP
#define FOE_YAML_POD_HPP

#include <foe/yaml/export.h>
#include <yaml-cpp/yaml.h>

#include <string>

#define POD_YAML_DECLARATION(T, Y)                                                                 \
    FOE_YAML_EXPORT bool yaml_read_##T(std::string const &nodeName, YAML::Node const &node,        \
                                       Y &data);                                                   \
                                                                                                   \
    FOE_YAML_EXPORT void yaml_write_##T(std::string const &nodeName, Y const &data,                \
                                        YAML::Node &node);

POD_YAML_DECLARATION(bool, bool)
POD_YAML_DECLARATION(string, std::string)

POD_YAML_DECLARATION(int, int)
POD_YAML_DECLARATION(int8_t, int8_t)
POD_YAML_DECLARATION(int16_t, int16_t)
POD_YAML_DECLARATION(int32_t, int32_t)
POD_YAML_DECLARATION(int64_t, int64_t)

POD_YAML_DECLARATION(unsigned_int, unsigned int)
POD_YAML_DECLARATION(uint8_t, uint8_t)
POD_YAML_DECLARATION(uint16_t, uint16_t)
POD_YAML_DECLARATION(uint32_t, uint32_t)
POD_YAML_DECLARATION(uint64_t, uint64_t)

POD_YAML_DECLARATION(float, float)
POD_YAML_DECLARATION(double, double)

#undef POD_YAML_DECLARATION

#endif // FOE_YAML_POD_HPP