// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IMPORTER_REGISTRATION_HPP
#define IMPORTER_REGISTRATION_HPP

#include <foe/ecs/id.h>
#include <foe/error_code.h>
#include <foe/imex/yaml/export.h>

class foeImporterBase;

FOE_IMEX_YAML_EXPORT foeResult foeImexYamlCreateImporter(foeIdGroup group,
                                                         char const *pFilesystemPath,
                                                         foeImporterBase **ppImporter);

#endif // IMPORTER_REGISTRATION_HPP