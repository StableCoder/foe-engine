/*
    Copyright (C) 2022 George Cave.

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

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "error_code.hpp"

void convertToDelimitedString(uint32_t srcCount,
                              char const **ppSrc,
                              uint32_t *pDstLength,
                              char **ppDst) {
    size_t totalLength = 0;
    for (size_t i = 0; i < srcCount; ++i) {
        // Length of the string plus space for the null character
        auto const len = strlen(ppSrc[i]);
        if (len > 0)
            totalLength += len + 1;
    }

    if (totalLength != 0) {
        char *pStr = new char[totalLength];

        *pDstLength = totalLength;
        *ppDst = pStr;

        for (size_t i = 0; i < srcCount; ++i) {
            memcpy(pStr, ppSrc[i], strlen(ppSrc[i]));
            pStr += strlen(ppSrc[i]);
            *pStr = '\0';
            ++pStr;
        }
    }
}

std::error_code getDelimitedString(uint32_t srcLength,
                                   char const *pSrc,
                                   uint32_t *pDstLength,
                                   char *pDst) {
    if (pDst == nullptr) {
        // If the destination pointer wasn't provided, then update and return the length required
        *pDstLength = srcLength;
        return FOE_GRAPHICS_VK_SUCCESS;
    }

    if (*pDstLength >= srcLength) {
        memcpy(pDst, pSrc, srcLength);
        *pDstLength = srcLength;

        return FOE_GRAPHICS_VK_SUCCESS;
    } else {
        uint32_t offset = *pDstLength;
        while (offset != 0 && pSrc[offset] != '\0') {
            --offset;
        }

        if (offset != 0) {
            // Increment by one to capture the ending null character
            ++offset;
            memcpy(pDst, pSrc, offset);
        }
        *pDstLength = offset;

        return FOE_GRAPHICS_VK_INCOMPLETE;
    }
}