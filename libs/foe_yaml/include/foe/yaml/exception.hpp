// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

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

    char const *what() const noexcept override;
    std::string whatStr() const noexcept;

  private:
    std::string mWhat;
};

#endif // FOE_YAML_EXCEPTION_HPP