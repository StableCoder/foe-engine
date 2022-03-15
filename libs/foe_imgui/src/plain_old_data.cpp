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

#include <foe/imgui/plain_old_data.hpp>

#include <fmt/core.h>
#include <foe/imgui/export.h>
#include <imgui.h>

#include <string>
#include <string_view>

template <typename T>
void imgui_pod(std::string const &label, T const &data) {
    std::string fmtStr = fmt::format("{}", data);
    ImGui::Text("%s: %s", label.c_str(), fmtStr.c_str());
}

#define INSTANTIATION(T)                                                                           \
    template FOE_IMGUI_EXPORT void imgui_pod<T>(std::string const &, T const &);

INSTANTIATION(bool)

INSTANTIATION(int8_t)
INSTANTIATION(int16_t)
INSTANTIATION(int32_t)
INSTANTIATION(int64_t)

INSTANTIATION(uint8_t)
INSTANTIATION(uint16_t)
INSTANTIATION(uint32_t)
INSTANTIATION(uint64_t)

INSTANTIATION(float)
INSTANTIATION(double)

INSTANTIATION(char const *)
INSTANTIATION(char *)
INSTANTIATION(std::string)
INSTANTIATION(std::string_view)
