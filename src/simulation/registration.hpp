// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BRINGUP_REGISTRATION_HPP
#define BRINGUP_REGISTRATION_HPP

#include <foe/error_code.h>

foeResult foeBringupRegisterFunctionality();

void foeBringupDeregisterFunctionality();

#endif // BRINGUP_REGISTRATION_HPP