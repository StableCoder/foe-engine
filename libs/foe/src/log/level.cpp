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

#include <foe/log/level.hpp>

namespace std {

std::string to_string(foeLogLevel logLevel) {
#define ADD_LEVEL(X)                                                                               \
    case foeLogLevel::X:                                                                           \
        return #X;

    switch (logLevel) {
        ADD_LEVEL(Fatal)
        ADD_LEVEL(Error)
        ADD_LEVEL(Warning)
        ADD_LEVEL(Info)
        ADD_LEVEL(Verbose)

    default:
        return "Unknown Level";
    }

#undef ADD_LEVEL
}

} // namespace std