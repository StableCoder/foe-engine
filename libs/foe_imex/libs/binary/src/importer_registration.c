// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/imex/binary/importer.h>

foeResultSet foeImexBinaryRegisterImporter() {
    return foeImexRegisterImporter(foeCreateBinaryImporter);
}

void foeImexBinaryDeregisterImporter() { foeImexDeregisterImporter(foeCreateBinaryImporter); }