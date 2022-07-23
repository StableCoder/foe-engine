// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_HANDLE_H
#define FOE_HANDLE_H

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

#define FOE_DEFINE_HANDLE_CASTS(HUMAN_NAME, OBJ_TYPE, HANDLE_TYPE)                                 \
                                                                                                   \
    inline OBJ_TYPE *HUMAN_NAME##_from_handle(HANDLE_TYPE handle) {                                \
        return reinterpret_cast<OBJ_TYPE *>(handle);                                               \
    }                                                                                              \
                                                                                                   \
    inline HANDLE_TYPE HUMAN_NAME##_to_handle(OBJ_TYPE *object) {                                  \
        return reinterpret_cast<HANDLE_TYPE>(object);                                              \
    }

#else

#define FOE_DEFINE_HANDLE_CASTS(HUMAN_NAME, OBJ_TYPE, HANDLE_TYPE)                                 \
                                                                                                   \
    inline OBJ_TYPE *HUMAN_NAME##_from_handle(HANDLE_TYPE handle) { return (OBJ_TYPE *)handle; }   \
                                                                                                   \
    inline HANDLE_TYPE HUMAN_NAME##_to_handle(OBJ_TYPE *object) { return (HANDLE_TYPE)object; }

#define FOE_DEFINE_STATIC_HANDLE_CASTS(HUMAN_NAME, OBJ_TYPE, HANDLE_TYPE)                          \
                                                                                                   \
    static inline OBJ_TYPE *HUMAN_NAME##_from_handle(HANDLE_TYPE handle) {                         \
        return (OBJ_TYPE *)handle;                                                                 \
    }                                                                                              \
                                                                                                   \
    static inline HANDLE_TYPE HUMAN_NAME##_to_handle(OBJ_TYPE *object) {                           \
        return (HANDLE_TYPE)object;                                                                \
    }

#endif

#endif // FOE_HANDLE_H