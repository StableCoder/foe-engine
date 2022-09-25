// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BINARY_FILE_HEADER_H
#define BINARY_FILE_HEADER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BinaryFileHeader {
    uint32_t dependencyDataOffset;

    uint32_t resourceIndexDataOffset;
    uint32_t resourceIndexDataSize;
    uint32_t entityIndexDataOffset;
    uint32_t entityIndexDataSize;

    uint32_t resourceEditorNamesOffset;
    uint32_t numResourceEditorNames;
    uint32_t entityEditorNamesOffset;
    uint32_t numEntityEditorNames;

    uint32_t resourceBinaryKeyIndexOffset;
    uint32_t resourceIndexOffset;
    uint32_t resourceDataOffset;

    uint32_t entityDataOffset;
    uint32_t fileDataOffset;
} BinaryFileHeader;

#ifdef __cplusplus
}
#endif

#endif // BINARY_FILE_HEADER_H