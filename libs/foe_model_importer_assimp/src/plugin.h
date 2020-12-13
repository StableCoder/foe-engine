/*
    Copyright (C) 2020 George Cave.

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

#ifndef PLUGIN2_HPP
#define PLUGIN2_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include <export.h>
#include <foe/plugin.h>

class foeModelFileImporterPlugin;

FOE_MODEL_IMPORTER_ASSIMP_EXPORT foeModelFileImporterPlugin *createModelFileImporterPlugin(
    foePlugin);

#ifdef __cplusplus
}
#endif

#endif // PLUGIN2_HPP