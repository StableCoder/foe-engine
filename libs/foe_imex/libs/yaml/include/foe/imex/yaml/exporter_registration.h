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

#ifndef FOE_IMEX_YAML_EXPORTER_REGISTRATION_H
#define FOE_IMEX_YAML_EXPORTER_REGISTRATION_H

#include <foe/error_code.h>
#include <foe/imex/yaml/export.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_IMEX_YAML_EXPORT foeResult foeImexYamlRegisterExporter();
FOE_IMEX_YAML_EXPORT void foeImexYamlDeregisterExporter();

#ifdef __cplusplus
}
#endif

#endif // FOE_IMEX_YAML_EXPORTER_REGISTRATION_H