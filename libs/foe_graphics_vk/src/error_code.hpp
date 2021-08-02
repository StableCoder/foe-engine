/*
    Copyright (C) 2021 George Cave.

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

#ifndef ERROR_CODE_HPP
#define ERROR_CODE_HPP

#include <system_error>

enum foeGraphicsVkResult {
    FOE_GRAPHICS_VK_SUCCESS = 0,
    // RenderTarget
    FOE_GRAPHICS_VK_ERROR_RENDER_TARGET_COULD_NOT_GET_COMPATIBLE_RENDER_PASS,
};

namespace std {
template <>
struct is_error_code_enum<foeGraphicsVkResult> : true_type {};
} // namespace std

std::error_code make_error_code(foeGraphicsVkResult);

#endif // ERROR_CODE_HPP