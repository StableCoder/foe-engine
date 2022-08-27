// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/position/compare.h>

#include <foe/position/component/3d.hpp>

extern "C" bool compare_foePosition3d(foePosition3d const *pData1, foePosition3d const *pData2) {
    // glm::vec3 - position
    if (pData1->position != pData2->position) {
        return false;
    }

    // glm::quat - orientation
    if (pData1->orientation != pData2->orientation) {
        return false;
    }

    return true;
}
