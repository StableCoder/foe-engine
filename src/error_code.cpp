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

struct foeBringupErrorCategory : std::error_category {
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *foeBringupErrorCategory::name() const noexcept { return "foeBringupResult"; }

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        return #X;

std::string foeBringupErrorCategory::message(int ev) const {
    switch (static_cast<foeBringupResult>(ev)) {
        RESULT_CASE(FOE_BRINGUP_SUCCESS)

        RESULT_CASE(FOE_BRINGUP_INITIALIZATION_FAILED)
        RESULT_CASE(FOE_BRINGUP_NOT_INITIALIZED)

        RESULT_CASE(FOE_BRINGUP_ERROR_FAILED_TO_REGISTER_ARMATURE_STATE_IMPORTER)
        RESULT_CASE(FOE_BRINGUP_ERROR_FAILED_TO_REGISTER_RENDER_STATE_IMPORTER)
        RESULT_CASE(FOE_BRINGUP_ERROR_FAILED_TO_REGISTER_CAMERA_IMPORTER)
        RESULT_CASE(FOE_BRINGUP_ERROR_FAILED_TO_REGISTER_EXPORTERS)

        RESULT_CASE(FOE_BRINGUP_ERROR_NO_PHYSICAL_DEVICE_MEETS_REQUIREMENTS)

        // RenderGraph - RenderScene
        RESULT_CASE(FOE_BRINGUP_RENDER_SCENE_COLOUR_TARGET_NOT_IMAGE)
        RESULT_CASE(FOE_BRINGUP_RENDER_SCENE_COLOUR_TARGET_NOT_MUTABLE)
        RESULT_CASE(FOE_BRINGUP_RENDER_SCENE_COLOUR_TARGET_NO_STATE)
        RESULT_CASE(FOE_BRINGUP_RENDER_SCENE_DEPTH_TARGET_NOT_IMAGE)
        RESULT_CASE(FOE_BRINGUP_RENDER_SCENE_DEPTH_TARGET_NOT_MUTABLE)
        RESULT_CASE(FOE_BRINGUP_RENDER_SCENE_DEPTH_TARGET_NO_STATE)

    default:
        if (ev > 0)
            return "(unrecognized positive foeBringupResult value)";
        else
            return "(unrecognized negative foeBringupResult value)";
    }
}

const foeBringupErrorCategory errorCategory{};

} // namespace

std::error_code make_error_code(foeBringupResult e) { return {static_cast<int>(e), errorCategory}; }