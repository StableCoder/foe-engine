// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/network/result.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foeNetworkResultToString(foeNetworkResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_NETWORK_SUCCESS)
        RESULT_CASE(FOE_NETWORK_INCOMPLETE)
        RESULT_CASE(FOE_NETWORK_STRING_INCOMPLETE)
        RESULT_CASE(FOE_NETWORK_ERROR_INVALID_PORT)
        RESULT_CASE(FOE_NETWORK_ERROR_NOT_IPV6)
        RESULT_CASE(FOE_NETWORK_ERROR_SOCKET_CREATION_ERROR)
        RESULT_CASE(FOE_NETWORK_ERROR_SOCKET_FAILED_TO_SET_DUAL_STACK)
        RESULT_CASE(FOE_NETWORK_ERROR_SOCKET_BIND_ERROR)
        RESULT_CASE(FOE_NETWORK_ERROR_WSA_STARTUP_FAILED)
        RESULT_CASE(FOE_NETWORK_ERROR_WSA_INCORRECT_VERSION)
        RESULT_CASE(FOE_NETWORK_ERROR_SEND_FAILURE)
        RESULT_CASE(FOE_NETWORK_ERROR_PARTIAL_SEND)
        RESULT_CASE(FOE_NETWORK_ERROR_RECIEVE_FAILURE)
        RESULT_CASE(FOE_NETWORK_ERROR_UNSUPPORTED_FAMILY_TYPE)
        RESULT_CASE(FOE_NETWORK_ERROR_SYSTEM_CALL_FAILED)

    default:
        if (value > 0) {
            sprintf(buffer, "FOE_NETWORK_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "FOE_NETWORK_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}