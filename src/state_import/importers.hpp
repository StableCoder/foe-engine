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

#ifndef IMPORTERS_HPP
#define IMPORTERS_HPP

#include <foe/ecs/id.hpp>

#include <filesystem>
#include <vector>

struct foeImporterBase;

class foeImporterGenerator {
  public:
    virtual auto createImporter(foeIdGroup group, std::filesystem::path stateDataPath)
        -> foeImporterBase * = 0;
};

class foeImporterFunctionRegistrar {
  public:
    virtual bool registerFunctions(foeImporterGenerator *pGenerator) = 0;
    virtual bool deregisterFunctions(foeImporterGenerator *pGenerator) = 0;
};

bool addImporterFunctionRegistrar(foeImporterFunctionRegistrar *pRegistrar);
bool removeImporterFunctionRegistrar(foeImporterFunctionRegistrar *pRegistrar);

bool addImporterGenerator(foeImporterGenerator *pGenerator);
bool removeImporterGenerator(foeImporterGenerator *pGenerator);

auto createImporter(foeIdGroup group, std::filesystem::path stateDataPath) -> foeImporterBase *;

#endif // IMPORTERS_HPP