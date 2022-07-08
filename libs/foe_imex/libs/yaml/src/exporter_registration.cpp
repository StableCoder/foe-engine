// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/imex/yaml/exporter_registration.h>

#include <foe/imex/exporters.hpp>
#include <foe/imex/yaml/exporter.hpp>

namespace {

std::string_view name{"Yaml"};
foeExporterVersion version{
    .major = 0,
    .minor = 0,
    .patch = 0,
};

} // namespace

extern "C" foeResult foeImexYamlRegisterExporter() {
    return foeImexRegisterExporter(foeExporter{
        .pName = name.data(),
        .version = version,
        .pExportFn = foeImexYamlExport,
    });
}

extern "C" void foeImexYamlDeregisterExporter() {
    foeImexDeregisterExporter(foeExporter{
        .pName = name.data(),
        .version = version,
        .pExportFn = foeImexYamlExport,
    });
}