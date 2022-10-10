// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/graphics/upload_buffer.h>
#include <foe/graphics/upload_context.h>
#include <foe/graphics/upload_request.h>

#include "create_test_session.hpp"

#include <cstring>

TEST_CASE() {
    foeResultSet result;
    foeGfxRuntime runtime = FOE_NULL_HANDLE;
    foeGfxSession session = FOE_NULL_HANDLE;
    foeGfxUploadContext uploadContext = FOE_NULL_HANDLE;
    foeGfxUploadBuffer uploadBuffer = FOE_NULL_HANDLE;

    result = createTestSession(&runtime, &session);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(runtime != FOE_NULL_HANDLE);
    REQUIRE(session != FOE_NULL_HANDLE);

    // Upload Context
    result = foeGfxCreateUploadContext(session, &uploadContext);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(uploadContext != FOE_NULL_HANDLE);

    // Upload Buffer
    result = foeGfxCreateUploadBuffer(uploadContext, 128, &uploadBuffer);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(uploadBuffer != FOE_NULL_HANDLE);

    void *pRawData = nullptr;
    result = foeGfxMapUploadBuffer(uploadContext, uploadBuffer, &pRawData);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(pRawData != nullptr);

    memset(pRawData, 0, 128);

    foeGfxUnmapUploadBuffer(uploadContext, uploadBuffer);

    // @todo Need to do foeGfxUploadRequest

    // Destroy
    // foeGfxDestroyUploadRequest(uploadContext, uploadRequest);
    foeGfxDestroyUploadBuffer(uploadContext, uploadBuffer);
    foeGfxDestroyUploadContext(uploadContext);

    foeGfxDestroySession(session);
    foeGfxDestroyRuntime(runtime);
}