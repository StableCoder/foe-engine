// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_NETWORK_RESULT_H
#define FOE_NETWORK_RESULT_H

#include <foe/network/export.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeNetworkResult {
    FOE_NETWORK_SUCCESS = 0,
    FOE_NETWORK_INCOMPLETE = 1000028001,
    FOE_NETWORK_STRING_INCOMPLETE = 1000028002,
    FOE_NETWORK_ERROR_INVALID_PORT = -1000028001,
    FOE_NETWORK_ERROR_NOT_IPV6 = -1000028002,
    FOE_NETWORK_ERROR_SOCKET_CREATION_ERROR = -1000028003,
    FOE_NETWORK_ERROR_SOCKET_FAILED_TO_SET_DUAL_STACK = -1000028004,
    FOE_NETWORK_ERROR_SOCKET_BIND_ERROR = -1000028005,
    FOE_NETWORK_ERROR_WSA_STARTUP_FAILED = -1000028006,
    FOE_NETWORK_ERROR_WSA_INCORRECT_VERSION = -1000028007,
    FOE_NETWORK_ERROR_SEND_FAILURE = -1000028008,
    FOE_NETWORK_ERROR_PARTIAL_SEND = -1000028009,
    FOE_NETWORK_ERROR_RECIEVE_FAILURE = -1000028010,
    FOE_NETWORK_ERROR_UNSUPPORTED_FAMILY_TYPE = -1000028011,
    FOE_NETWORK_ERROR_SYSTEM_CALL_FAILED = -1000028012,
} foeNetworkResult;

FOE_NETWORK_EXPORT
void foeNetworkResultToString(foeNetworkResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_NETWORK_RESULT_H