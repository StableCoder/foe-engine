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

#include <foe/imex/importers.hpp>

#include <mutex>
#include <vector>

namespace {

std::mutex sync;

std::vector<foeImporterFunctionRegistrar *> registrars;
std::vector<foeImporterGenerator *> generators;

} // namespace

bool addImporterFunctionRegistrar(foeImporterFunctionRegistrar *pRegistrar) {
    std::scoped_lock lock{sync};

    for (auto const &it : registrars) {
        if (it == pRegistrar)
            return false;
    }

    // Add the registrar
    registrars.emplace_back(pRegistrar);

    // Run the registrar on all available generators to register where possible
    for (auto &it : generators) {
        pRegistrar->registerFunctions(it);
    }

    return true;
}

bool removeImporterFunctionRegistrar(foeImporterFunctionRegistrar *pRegistrar) {
    std::scoped_lock lock{sync};

    for (auto it = registrars.begin(); it != registrars.end(); ++it) {
        if (*it != pRegistrar)
            continue;

        // Found, deregister functions from associated generators
        for (auto &pGenerator : generators) {
            pRegistrar->deregisterFunctions(pGenerator);
        }

        registrars.erase(it);
    }

    return false;
}

bool addImporterGenerator(foeImporterGenerator *pGenerator) {
    std::scoped_lock lock{sync};

    for (auto const &it : generators) {
        if (it == pGenerator)
            return false;
    }

    // Add the generator
    generators.emplace_back(pGenerator);

    // Run all current registrars to it
    for (auto pRegistrar : registrars) {
        pRegistrar->registerFunctions(pGenerator);
    }

    return true;
}

bool removeImporterGenerator(foeImporterGenerator *pGenerator) {
    std::scoped_lock lock{sync};

    for (auto it = generators.begin(); it != generators.end(); ++it) {
        if (*it != pGenerator)
            continue;

        // Found, remove it
        generators.erase(it);

        return true;
    }

    return false;
}

auto createImporter(foeIdGroup group, std::filesystem::path stateDataPath) -> foeImporterBase * {
    std::scoped_lock lock{sync};

    for (auto it : generators) {
        auto *pImporter = it->createImporter(group, stateDataPath);
        if (pImporter != nullptr)
            return pImporter;
    }

    return nullptr;
}