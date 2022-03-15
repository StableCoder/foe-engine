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