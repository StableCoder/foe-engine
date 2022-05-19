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

#ifndef FOE_ERROR_CODE_H
#define FOE_ERROR_CODE_H

#ifdef __cplusplus
#include <system_error>
#endif

/// Global value for an unqualified 'SUCCESS' result code
#define FOE_SUCCESS 0
/// The minimum possible result code (not quite the full rangle to allow conversion to the absolute
/// value)
#define FOE_RESULT_MIN_ENUM -0x7FFFFFFF
/// The maximum possible result code
#define FOE_RESULT_MAX_ENUM 0x7FFFFFFF
/// The maximum length that a result code name that can be returned from a
/// <result>ToString(<result>) function
#define FOE_MAX_RESULT_STRING_SIZE 128

/** @brief Type to transport std::error_code data across C-API boundaries
 *
 * The value/category combination is too useful to pass up. As such, this simple struct type can be
 * used to transport these types through C-APIs while retaining all the information needed to easily
 * reconstruct the std::error_code later.
 *
 * In C, only the simple struct is available.
 *
 * In C++, there are several constructors, including one to build straight from a std::error_code
 * type, or vice-versa implicitly.
 */
typedef struct foeErrorCode {
    int value;
    void const *pCategory;
} foeErrorCode;

inline foeErrorCode foeToErrorCode(int value, void const *pCategory) {
    foeErrorCode errC = {
        .value = value,
        .pCategory = pCategory,
    };

    return errC;
}

#ifdef __cplusplus
inline foeErrorCode foeToErrorCode(std::error_code errC) {
    return foeErrorCode{
        .value = errC.value(),
        .pCategory = reinterpret_cast<void const *>(&errC.category()),
    };
}

namespace std {
template <>
struct is_error_code_enum<foeErrorCode> : true_type {};
} // namespace std

/// In C++, can implicitly convert the foeErrorCode to std::error_code without issue
inline std::error_code make_error_code(foeErrorCode e) {
    return {e.value, *static_cast<std::error_category const *>(e.pCategory)};
}
#endif

#endif // FOE_ERROR_CODE_H