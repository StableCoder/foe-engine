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

#ifndef FOE_HANDLE_H
#define FOE_HANDLE_H

#ifdef __cplusplus
extern "C" {
#endif

// Based on the definition of handle types in Vulkan, sets declarable handle types that are always
// 64 bits large.
#if !defined(FOE_DEFINE_HANDLE)
#if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__)) ||        \
    defined(_M_X64) || defined(__ia64) || defined(_M_IA64) || defined(__aarch64__) ||              \
    defined(__powerpc64__)
#define FOE_DEFINE_HANDLE(object) typedef struct object##_T *object;
#else
#define FOE_DEFINE_HANDLE(object) typedef uint64_t object;
#endif
#endif

// A null value for FoE handle types
#define FOE_NULL_HANDLE 0

#ifdef __cplusplus
}
#endif

#endif // FOE_HANDLE_H