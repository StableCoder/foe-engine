// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/hex.h>

#include "result.h"

#include <stdint.h>

foeResultSet foeEncodeHex(size_t dataSize,
                          void const *pData,
                          size_t hexBufferSize,
                          char *pHexBuffer) {
    if (hexBufferSize < (dataSize * 2) + 1) {
        return to_foeResult(FOE_ERROR_DESTINATION_BUFFER_TOO_SMALL);
    }

    uint8_t const *pSrc = pData;
    for (; dataSize > 0; --dataSize) {
        uint8_t byte = *pSrc++;

        *pHexBuffer++ = "0123456789ABCDEF"[byte >> 4];
        *pHexBuffer++ = "0123456789ABCDEF"[byte & 15];
    }
    *pHexBuffer = '\0';

    return to_foeResult(FOE_SUCCESS);
}

static inline uint8_t decodeHexCharacter(char character) {
    switch (character) {
    case '0':
        return 0x0;
    case '1':
        return 0x1;
    case '2':
        return 0x2;
    case '3':
        return 0x3;
    case '4':
        return 0x4;
    case '5':
        return 0x5;
    case '6':
        return 0x6;
    case '7':
        return 0x7;
    case '8':
        return 0x8;
    case '9':
        return 0x9;
    case 'A':
    case 'a':
        return 0xA;
    case 'B':
    case 'b':
        return 0xB;
    case 'C':
    case 'c':
        return 0xC;
    case 'D':
    case 'd':
        return 0xD;
    case 'E':
    case 'e':
        return 0xE;
    case 'F':
    case 'f':
        return 0xF;
    default:
        return 0xFF;
    }
}

foeResultSet foeDecodeHex(size_t hexDataSize,
                          char const *pHexData,
                          size_t *pDataSize,
                          void *pData) {
    // If HEX data size is odd, that would mean that there is a remaining half-byte?
    // Don't support that
    if ((hexDataSize & 0x1) != 0)
        return to_foeResult(FOE_ERROR_INVALID_HEX_DATA_SIZE);

    // Each character in a HEX string represents a half-byte, so the destination
    // must be half the size of the total buffer
    if (*pDataSize < hexDataSize / 2)
        return to_foeResult(FOE_ERROR_DESTINATION_BUFFER_TOO_SMALL);

    uint8_t *pDecodedData = pData;
    uint32_t decodedDataSize = 0;
    for (; hexDataSize > 1; hexDataSize -= 2) {
        // First HEX in pair
        uint8_t firstHex = decodeHexCharacter(*pHexData++);
        if (firstHex == 0xFF)
            return to_foeResult(FOE_ERROR_MALFORMED_HEX_DATA);

        uint8_t secondHex = decodeHexCharacter(*pHexData++);
        if (secondHex == 0xFF)
            return to_foeResult(FOE_ERROR_MALFORMED_HEX_DATA);

        *pDecodedData++ = (firstHex << 4) | secondHex;
        ++decodedDataSize;
    }

    *pDataSize = decodedDataSize;

    return to_foeResult(FOE_SUCCESS);
}