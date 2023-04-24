// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/imgui/plain_old_data.hpp>

#include <fmt/core.h>
#include <foe/imgui/export.h>
#include <imgui.h>

#include <cstdint>
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
