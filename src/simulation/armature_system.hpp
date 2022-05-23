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

#ifndef ARMATURE_SYSTEM_HPP
#define ARMATURE_SYSTEM_HPP

#include <foe/resource/pool.h>

class foeArmatureStatePool;

class foeArmatureSystem {
  public:
    foeResult initialize(foeResourcePool resourcePool, foeArmatureStatePool *pArmatureStatePool);
    void deinitialize();
    bool initialized() const noexcept;

    void process(float timePassed);

  private:
    // Resources
    foeResourcePool mResourcePool{FOE_NULL_HANDLE};

    // Components
    foeArmatureStatePool *mpArmatureStatePool{nullptr};
};

#endif // ARMATURE_SYSTEM_HPP