// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RENDER_STATE_POOL_HPP
#define RENDER_STATE_POOL_HPP

#include <foe/data_pool.hpp>

#include "render_state.hpp"
#include "type_defs.h"

class foeRenderStatePool : public foeDataPool<foeEntityID, foeRenderState> {
  public:
    void maintenance() { foeDataPool<foeEntityID, foeRenderState>::maintenance(); }
};

#endif // RENDER_STATE_POOL_HPP