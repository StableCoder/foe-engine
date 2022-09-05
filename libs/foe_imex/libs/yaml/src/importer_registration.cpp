// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/imex/yaml/importer_registration.h>

#include <foe/imex/importer.h>
#include <foe/imex/yaml/importer.hpp>

#include "common.hpp"
#include "log.hpp"
#include "result.h"

extern "C" foeResultSet foeImexYamlRegisterImporter() {
    return foeImexRegisterImporter(&foeCreateYamlImporter);
}

extern "C" void foeImexYamlDeregisterImporter() {
    foeImexDeregisterImporter(&foeCreateYamlImporter);
}