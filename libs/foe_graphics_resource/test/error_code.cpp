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

#include <catch.hpp>

#include "../src/error_code.hpp"

#define ERROR_CODE_CATCH_CHECK(X)                                                                  \
    SECTION(#X) {                                                                                  \
        errC = X;                                                                                  \
                                                                                                   \
        CHECK(errC.value() == X);                                                                  \
        CHECK(errC.message() == #X);                                                               \
        CHECK(std::string_view{errC.category().name()} == "foeGraphicsResourceResult");            \
    }

TEST_CASE("Ensure error codes return correct values and strings") {
    std::error_code errC;

    SECTION("Generic non-existant negative value") {
        errC = static_cast<foeGraphicsResourceResult>(FOE_RESULT_MIN_ENUM);

        CHECK(errC.value() == FOE_RESULT_MIN_ENUM);
        CHECK(errC.message() == "FOE_GRAPHICS_RESOURCE_UNKNOWN_ERROR_2147483647");
    }
    SECTION("Generic non-existant positive value") {
        errC = static_cast<foeGraphicsResourceResult>(FOE_RESULT_MAX_ENUM);

        CHECK(errC.value() == FOE_RESULT_MAX_ENUM);
        CHECK(errC.message() == "FOE_GRAPHICS_RESOURCE_UNKNOWN_SUCCESS_2147483647");
    }

    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_RESOURCE_SUCCESS)
    // Loaders
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_RESOURCE_ERROR_INCOMPATIBLE_CREATE_INFO)
    // Image Loader
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_RESOURCE_ERROR_IMAGE_LOADER_INITIALIZATION_FAILED)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_RESOURCE_ERROR_IMAGE_LOADER_NOT_INITIALIZED)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_RESOURCE_ERROR_EXTERNAL_IMAGE_FORMAT_UNKNOWN)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_RESOURCE_ERROR_EXTERNAL_IMAGE_LOAD_FAILURE)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_RESOURCE_ERROR_IMAGE_UPLOAD_FAILURE)
    // Material Loader
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_LOADER_INITIALIZATION_FAILED)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_LOADER_NOT_INITIALIZED)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_SUBRESOURCE_FAILED_TO_LOAD)
    // Mesh Loader
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_RESOURCE_ERROR_MESH_LOADER_INITIALIZATION_FAILED)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_RESOURCE_ERROR_MESH_LOADER_NOT_INITIALIZED)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_RESOURCE_ERROR_MESH_UPLOAD_FAILED)
    // Vertex Descriptor Loader
    ERROR_CODE_CATCH_CHECK(
        FOE_GRAPHICS_RESOURCE_ERROR_VERTEX_DESCRIPTOR_LOADER_INITIALIZATION_FAILED)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_RESOURCE_ERROR_VERTEX_DESCRIPTOR_SUBRESOURCE_FAILED_TO_LOAD)
    // Shader Loader
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_RESOURCE_ERROR_SHADER_LOADER_INITIALIZATION_FAILED)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_RESOURCE_ERROR_SHADER_LOADER_NOT_INITIALIZED)
    ERROR_CODE_CATCH_CHECK(FOE_GRAPHICS_RESOURCE_ERROR_SHADER_LOADER_BINARY_FILE_NOT_FOUND)
}