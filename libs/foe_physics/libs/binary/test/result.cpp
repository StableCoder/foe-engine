// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/physics/binary/result.h>

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        foePhysicsBinaryResultToString(X, resultString);                                           \
        CHECK(std::string{resultString} == #X);                                                    \
    }

TEST_CASE("foePhysicsBinaryResult - Ensure error codes return correct values and strings") {
    char resultString[FOE_MAX_RESULT_STRING_SIZE];

    SECTION("Generic non-existant negative value") {
        foePhysicsBinaryResultToString((foePhysicsBinaryResult)FOE_RESULT_MIN_ENUM, resultString);
        CHECK(std::string{resultString} == "FOE_PHYSICS_BINARY_UNKNOWN_ERROR_2147483647");
    }
    SECTION("Generic non-existant positive value") {
        foePhysicsBinaryResultToString((foePhysicsBinaryResult)FOE_RESULT_MAX_ENUM, resultString);
        CHECK(std::string{resultString} == "FOE_PHYSICS_BINARY_UNKNOWN_SUCCESS_2147483647");
    }

    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_BINARY_SUCCESS)
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_BINARY_DATA_NOT_EXPORTED)
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_BINARY_ERROR_NO_CREATE_INFO_PROVIDED)
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_BINARY_ERROR_OUT_OF_MEMORY)
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_BINARY_ERROR_FAILED_TO_REGISTER_COLLISION_SHAPE_IMPORTER)
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_BINARY_ERROR_FAILED_TO_REGISTER_COLLISION_SHAPE_EXPORTER)
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_BINARY_ERROR_COLLISION_SHAPE_POOL_NOT_FOUND)
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_BINARY_ERROR_COLLISION_SHAPE_ALREADY_EXISTS)
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_BINARY_ERROR_FAILED_TO_REGISTER_RIGID_BODY_IMPORTER)
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_BINARY_ERROR_FAILED_TO_REGISTER_RIGID_BODY_EXPORTER)
    ERROR_CODE_CATCH_CHECK(FOE_PHYSICS_BINARY_ERROR_RIGID_BODY_POOL_NOT_FOUND)
}
