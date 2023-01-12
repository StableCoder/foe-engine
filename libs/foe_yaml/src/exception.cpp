// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/yaml/exception.hpp>

foeYamlException::foeYamlException(std::string what) : mWhat{what} {}

foeYamlException::operator std::string() const noexcept { return mWhat; }

char const *foeYamlException::what() const noexcept { return mWhat.data(); }

std::string foeYamlException::whatStr() const noexcept { return mWhat; }
