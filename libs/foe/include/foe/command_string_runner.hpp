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

#ifndef FOE_COMMAND_STRING_RUNNER_HPP
#define FOE_COMMAND_STRING_RUNNER_HPP

#include <foe/export.h>

#include <functional>
#include <mutex>
#include <string_view>
#include <unordered_map>

class foeCommandStringRunner {
  public:
    FOE_EXPORT bool registerCommand(std::string_view commandName,
                                    std::function<void(std::string_view)> const &function);

    FOE_EXPORT bool deregisterCommand(std::string_view commandName);

    FOE_EXPORT bool runCommand(std::string_view commandCall);

  private:
    std::mutex mSync;
    std::unordered_map<std::string, std::function<void(std::string_view)>> mCommandMap;
};

#endif // FOE_COMMAND_STRING_RUNNER_HPP