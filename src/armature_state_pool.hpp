/*
    Copyright (C) 2021-2022 George Cave.

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

#ifndef ARMATURE_STATE_POOL_HPP
#define ARMATURE_STATE_POOL_HPP

#include <foe/data_pool.hpp>
#include <foe/simulation/core/component_pool_base.hpp>

#include "armature_state.hpp"
#include "type_defs.h"

class foeArmatureStatePool : public foeComponentPoolBase,
                             public foeDataPool<foeEntityID, foeArmatureState> {
  public:
    foeArmatureStatePool() : foeComponentPoolBase{FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL} {}

    void maintenance() override { foeDataPool<foeEntityID, foeArmatureState>::maintenance(); }
};

#endif // ARMATURE_STATE_POOL_HPP