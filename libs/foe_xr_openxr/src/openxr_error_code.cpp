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

#include <foe/xr/error_code.hpp>

#include <openxr/openxr_reflection.h>

namespace {

#define XR_ENUM_CASE_STR(name, val)                                                                \
    case name:                                                                                     \
        return #name;

#define XR_ENUM_STR(enumType)                                                                      \
    constexpr const char *XrEnumStr(enumType ev) {                                                 \
        switch (ev) {                                                                              \
            XR_LIST_ENUM_##enumType(XR_ENUM_CASE_STR);                                             \
        default:                                                                                   \
            if (ev > 0)                                                                            \
                return "(unrecognized positive XrResult value)";                                   \
            else                                                                                   \
                return "(unrecognized negative XrResult value)";                                   \
        }                                                                                          \
    }

XR_ENUM_STR(XrResult);

struct XrErrCategory : std::error_category {
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *XrErrCategory::name() const noexcept { return "XrResult"; }

std::string XrErrCategory::message(int ev) const { return XrEnumStr(static_cast<XrResult>(ev)); }

const XrErrCategory XrErrCategory{};

} // namespace

std::error_code make_error_code(XrResult e) { return {static_cast<int>(e), XrErrCategory}; }