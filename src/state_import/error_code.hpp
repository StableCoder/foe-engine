/*
    Copyright (C) 2021-2022 George Cave.

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

enum foeStateImportResult : int {
    FOE_STATE_IMPORT_SUCCESS = 0,
    FOE_STATE_IMPORT_ERROR_NO_IMPORTER,
    FOE_STATE_IMPORT_ERROR_IMPORTING_DEPENDENCIES,
    FOE_STATE_IMPORT_ERROR_DUPLICATE_DEPENDENCIES,
    FOE_STATE_IMPORT_ERROR_TRANSITIVE_DEPENDENCIES_UNFULFILLED,
    FOE_STATE_IMPORT_ERROR_ECS_GROUP_SETUP_FAILURE,
    FOE_STATE_IMPORT_ERROR_IMPORTING_INDEX_DATA,
    FOE_STATE_IMPORT_ERROR_IMPORTING_RESOURCE,
    FOE_STATE_IMPORT_ERROR_NO_COMPONENT_IMPORTER,
};

namespace std {
template <>
struct is_error_code_enum<foeStateImportResult> : true_type {};
} // namespace std

std::error_code make_error_code(foeStateImportResult);

#endif // ERROR_CODE_HPP