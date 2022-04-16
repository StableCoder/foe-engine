/*
    Copyright (C) 2022 George Cave.

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

#ifndef IMPORTER_REGISTRATION_HPP
#define IMPORTER_REGISTRATION_HPP

#include <foe/ecs/id.h>
#include <foe/error_code.h>
#include <foe/imex/yaml/export.h>

class foeImporterBase;

FOE_IMEX_YAML_EXPORT foeErrorCode foeImexYamlCreateImporter(foeIdGroup group,
                                                            char const *pFilesystemPath,
                                                            foeImporterBase **ppImporter);

#endif // IMPORTER_REGISTRATION_HPP