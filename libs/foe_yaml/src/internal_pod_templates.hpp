// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef INTERNAL_POD_TEMPLATES_HPP
#define INTERNAL_POD_TEMPLATES_HPP

#include <yaml-cpp/yaml.h>

#include <string>

template <typename T>
bool yaml_read(std::string const &nodeName, YAML::Node const &node, T &data);

template <typename T>
void yaml_write(std::string const &nodeName, T const &data, YAML::Node &node);

#endif // INTERNAL_POD_TEMPLATES_HPP