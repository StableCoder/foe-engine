// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "armature_state.hpp"

void cleanup_foeArmatureState(foeArmatureState const *pData) { free(pData->pArmatureBones); }