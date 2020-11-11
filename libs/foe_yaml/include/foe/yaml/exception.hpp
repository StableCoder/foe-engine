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

#ifndef FOE_YAML_EXCEPTION_HPP
#define FOE_YAML_EXCEPTION_HPP

#include <foe/yaml/export.h>

#include <exception>
#include <string>

/// Exceptions that stores both the nodes and the reason for failure
class FOE_YAML_EXPORT foeYamlException : std::exception {
  public:
    /** @brief String constructor
     * @param what Why the exception is being thrown
     */
    foeYamlException(std::string what);

    operator std::string() const noexcept;

    const char *what() const noexcept override;
    std::string whatStr() const noexcept;

  private:
    std::string mWhat;
};

#endif // FOE_YAML_EXCEPTION_HPP