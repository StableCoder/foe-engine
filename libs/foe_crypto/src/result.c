// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/crypto/result.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foeCryptoResultToString(foeCryptoResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_CRYPTO_SUCCESS)
        RESULT_CASE(FOE_CRYPTO_ERROR_OUT_OF_MEMORY)
        RESULT_CASE(FOE_CRYPTO_ERROR_LIBRARY_FAILED_TO_INITIALIZE)
        RESULT_CASE(FOE_CRYPTO_ERROR_INITIALIZATION_FAILED)
        RESULT_CASE(FOE_CRYPTO_ERROR_NO_AES_HARDWARE_ACCELERATION_AVAILABLE)
        RESULT_CASE(FOE_CRYPTO_ERROR_INVALID_KEY_SIZE)
        RESULT_CASE(FOE_CRYPTO_ERROR_INVALID_NONCE_SIZE)
        RESULT_CASE(FOE_CRYPTO_ERROR_FAILED_TO_CREATE_KEY_PAIR)
        RESULT_CASE(FOE_CRYPTO_ERROR_FAILED_TO_PERFORM_KEY_EXCHANGE)
        RESULT_CASE(FOE_CRYPTO_ERROR_DESTINATION_BUFFER_TOO_SMALL)
        RESULT_CASE(FOE_CRYPTO_ERROR_FAILED_TO_SIGN)
        RESULT_CASE(FOE_CRYPTO_ERROR_FAILED_TO_VERIFY)
        RESULT_CASE(FOE_CRYPTO_ERROR_FAILED_TO_ENCRYPT)
        RESULT_CASE(FOE_CRYPTO_ERROR_FAILED_TO_DECRYPT)

    default:
        if (value > 0) {
            sprintf(buffer, "FOE_CRYPTO_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "FOE_CRYPTO_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}