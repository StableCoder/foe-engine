// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_IMGUI_PLAIN_OLD_DATA_HPP
#define FOE_IMGUI_PLAIN_OLD_DATA_HPP

#include <string>

/**
 * @brief Displays the templated type using ImGui, with the given label on basic types
 * @tparam Type that the templated function will display the details of
 * @param label On simple types, such a plain-old-data, this will be prefixed to the content, such
 * as 'label: value'
 * @param data The content to be actually displayed
 */
template <typename T>
void imgui_pod(std::string const &label, T const &data);

#endif // FOE_IMGUI_PLAIN_OLD_DATA_HPP