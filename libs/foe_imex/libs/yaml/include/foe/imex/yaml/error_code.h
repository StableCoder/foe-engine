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

#ifndef FOE_IMEX_YAML_ERROR_CODE_H
#define FOE_IMEX_YAML_ERROR_CODE_H

#ifdef __cplusplus
extern "C" {
#endif

enum foeImexYamlResult {
    FOE_IMEX_YAML_SUCCESS = 0,
    FOE_IMEX_YAML_ERROR_FUNCTIONALITY_ALREADY_REGISTERED,
    FOE_IMEX_YAML_ERROR_FUNCTIONALITY_NOT_REGISTERED,
    // Exporter
    FOE_IMEX_YAML_ERROR_EXPORTER_ALREADY_REGISTERED,
    FOE_IMEX_YAML_ERROR_EXPORTER_NOT_REGISTERED,
    FOE_IMEX_YAML_ERROR_DESTINATION_NOT_DIRECTORY,
    FOE_IMEX_YAML_ERROR_FAILED_TO_PERFORM_FILESYSTEM_OPERATION,
    FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_DEPENDENCIES,
    FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_RESOURCE_INDEX_DATA,
    FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_COMPONENT_INDEX_DATA,
    FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_RESOURCE_DATA,
    FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_COMPONENT_DATA
};

#ifdef __cplusplus
}
#endif

#endif // FOE_IMEX_YAML_ERROR_CODE_H