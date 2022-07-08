// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IMPORTERS_HPP
#define IMPORTERS_HPP

#include <foe/ecs/id.h>
#include <foe/error_code.h>
#include <foe/imex/export.h>

class foeImporterBase;

typedef foeResult (*PFN_foeImexCreateImporter)(foeIdGroup, char const *, foeImporterBase **);

FOE_IMEX_EXPORT foeResult foeImexRegisterImporter(PFN_foeImexCreateImporter createImporter);
FOE_IMEX_EXPORT foeResult foeImexDeregisterImporter(PFN_foeImexCreateImporter createImporter);

#include <filesystem>

FOE_IMEX_EXPORT auto createImporter(foeIdGroup group, std::filesystem::path stateDataPath)
    -> foeImporterBase *;

#endif // IMPORTERS_HPP