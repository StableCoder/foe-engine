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

struct foeGraphicsResourceYamlErrorCategory : std::error_category {
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *foeGraphicsResourceYamlErrorCategory::name() const noexcept {
    return "foeGraphicsResourceYamlResult";
}

std::string foeGraphicsResourceYamlErrorCategory::message(int ev) const {
    char buffer[FOE_MAX_RESULT_STRING_SIZE];
    foeGraphicsResourceYamlResultToString((foeGraphicsResourceYamlResult)ev, buffer);
    return buffer;
}

const foeGraphicsResourceYamlErrorCategory errorCategory{};

} // namespace

std::error_code make_error_code(foeGraphicsResourceYamlResult e) {
    return {static_cast<int>(e), errorCategory};
}