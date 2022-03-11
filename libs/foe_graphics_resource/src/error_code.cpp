/*
    Copyright (C) 2021-2022 George Cave.

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

#include "error_code.hpp"

namespace {

struct foeGraphicsResourceErrCategory : std::error_category {
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *foeGraphicsResourceErrCategory::name() const noexcept {
    return "foeGraphicsResourceResult";
}

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        return #X;

std::string foeGraphicsResourceErrCategory::message(int ev) const {
    switch (static_cast<foeGraphicsResourceResult>(ev)) {
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_SUCCESS)
        // Loaders
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_FAILED_TO_FIND_COMPATIBLE_LOADER)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_INCOMPATIBLE_CREATE_INFO)
        // Image Loader
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_IMAGE_LOADER_INITIALIZATION_FAILED)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_IMAGE_LOADER_NOT_INITIALIZED)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_EXTERNAL_IMAGE_FORMAT_UNKNOWN)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_EXTERNAL_IMAGE_LOAD_FAILURE)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_IMAGE_UPLOAD_FAILURE)
        // Material Loader
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_LOADER_INITIALIZATION_FAILED)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_LOADER_NOT_INITIALIZED)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_SUBRESOURCE_FAILED_TO_LOAD)
        // Mesh Loader
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_MESH_LOADER_INITIALIZATION_FAILED)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_MESH_LOADER_NOT_INITIALIZED)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_MESH_UPLOAD_FAILED)
        // Vertex Descriptor Loader
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_VERTEX_DESCRIPTOR_LOADER_INITIALIZATION_FAILED)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_VERTEX_DESCRIPTOR_SUBRESOURCE_FAILED_TO_LOAD)
        // Shader Loader
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_SHADER_LOADER_INITIALIZATION_FAILED)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_SHADER_LOADER_NOT_INITIALIZED)
        RESULT_CASE(FOE_GRAPHICS_RESOURCE_ERROR_SHADER_LOADER_BINARY_FILE_NOT_FOUND)

    default:
        if (ev > 0)
            return "(unrecognized positive foeGraphicsResourceResult value)";
        else
            return "(unrecognized negative foeGraphicsResourceResult value)";
    }
}

const foeGraphicsResourceErrCategory graphicsResourceErrCategory{};

} // namespace

std::error_code make_error_code(foeGraphicsResourceResult e) {
    return {static_cast<int>(e), graphicsResourceErrCategory};
}