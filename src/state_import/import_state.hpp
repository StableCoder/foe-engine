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

#ifndef IMPORT_STATE_HPP
#define IMPORT_STATE_HPP

#include <filesystem>

class foeSearchPaths;
struct SimulationSet;

auto importState(std::filesystem::path stateDataPath,
                 foeSearchPaths *pSearchPaths,
                 SimulationSet **ppSimulationSet) -> std::error_code;

#endif // IMPORT_STATE_HPP