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