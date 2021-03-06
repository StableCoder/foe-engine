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

#ifndef FOE_RESOURCE_ERROR_CODE_HPP
#define FOE_RESOURCE_ERROR_CODE_HPP

#include <foe/resource/export.h>

#include <system_error>

enum foeResourceResult : int {
    FOE_RESOURCE_SUCCESS = 0,
    FOE_RESOURCE_ERROR_ALREADY_INITIALIZED,
    FOE_RESOURCE_ERROR_IMPORT_FAILED,
};

namespace std {
template <>
struct is_error_code_enum<foeResourceResult> : true_type {};
} // namespace std

FOE_RES_EXPORT std::error_code make_error_code(foeResourceResult);

#endif // FOE_RESOURCE_ERROR_CODE_HPP