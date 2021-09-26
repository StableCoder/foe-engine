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

#include <foe/imex/yaml/exporter2.hpp>

#include <foe/imex/exporters.hpp>
#include <foe/imex/yaml/exporter.hpp>

namespace {

std::string_view name{"Yaml"};
foeExporterVersion version{
    .major = 0,
    .minor = 0,
    .patch = 0,
};

std::error_code exportData(std::filesystem::path path, foeSimulationState *pSimState) {
    foeYamlExporter exporter;
    exporter.exportState(path, pSimState);

    return std::error_code{};
}

} // namespace

auto foeImexYamlRegisterExporter() -> std::error_code {
    return foeImexRegisterExporter(foeExporter{
        .pName = name.data(),
        .version = version,
        .pExportFn = exportData,
    });
}

auto foeImexYamlDeregisterExporter() -> std::error_code {
    return foeImexDeregisterExporter(foeExporter{
        .pName = name.data(),
        .version = version,
        .pExportFn = exportData,
    });
}