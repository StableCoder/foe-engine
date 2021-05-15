/*
    Copyright (C) 2021 George Cave.

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

struct foeStateImportErrorCategory : std::error_category {
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *foeStateImportErrorCategory::name() const noexcept { return "foeStateImportResult"; }

std::string foeStateImportErrorCategory::message(int ev) const {
    switch (static_cast<foeStateImportResult>(ev)) {
    case FOE_STATE_IMPORT_SUCCESS:
        return "FOE_STATE_IMPORT_SUCCESS";

    case FOE_STATE_IMPORT_ERROR_NO_IMPORTER:
        return "FOE_STATE_IMPORT_ERROR_NO_IMPORTER";

    case FOE_STATE_IMPORT_ERROR_IMPORTING_DEPENDENCIES:
        return "FOE_STATE_IMPORT_ERROR_IMPORTING_DEPENDENCIES";

    case FOE_STATE_IMPORT_ERROR_DUPLICATE_DEPENDENCIES:
        return "FOE_STATE_IMPORT_ERROR_DUPLICATE_DEPENDENCIES";

    case FOE_STATE_IMPORT_ERROR_TRANSITIVE_DEPENDENCIES_UNFULFILLED:
        return "FOE_STATE_IMPORT_ERROR_TRANSITIVE_DEPENDENCIES_UNFULFILLED";

    case FOE_STATE_IMPORT_ERROR_ECS_GROUP_SETUP_FAILURE:
        return "FOE_STATE_IMPORT_ERROR_ECS_GROUP_SETUP_FAILURE";

    case FOE_STATE_IMPORT_ERROR_IMPORTING_INDEX_DATA:
        return "FOE_STATE_IMPORT_ERROR_IMPORTING_INDEX_DATA";

    case FOE_STATE_IMPORT_ERROR_IMPORTING_RESOURCE:
        return "FOE_STATE_IMPORT_ERROR_IMPORTING_RESOURCE";

    default:
        if (ev > 0)
            return "(unrecognized positive foeStateImportResult value)";
        else
            return "(unrecognized negative foeStateImportResult value)";
    }
}

const foeStateImportErrorCategory importStateErrorCategory{};

} // namespace

std::error_code make_error_code(foeStateImportResult e) {
    return {static_cast<int>(e), importStateErrorCategory};
}