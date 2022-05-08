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

#include "armature_system.hpp"

#include <foe/resource/pool.h>

#include "../error_code.hpp"
#include "armature.hpp"
#include "armature_state.hpp"
#include "armature_state_pool.hpp"
#include "type_defs.h"

namespace {

void originalArmatureNode(foeArmatureNode const *pNode,
                          glm::mat4 const &parentTransform,
                          glm::mat4 *pArmatureTransform) {

    *pArmatureTransform = parentTransform * pNode->transformMatrix;

    for (size_t child = 0; child < pNode->numChildren; ++child) {
        size_t const offset = pNode->childrenOffset + child;

        originalArmatureNode(pNode + offset, *pArmatureTransform, pArmatureTransform + offset);
    }
}

void animateArmatureNode(foeArmatureNode const *pNode,
                         std::vector<foeNodeAnimationChannel> const &animationChannels,
                         double const animationTick,
                         glm::mat4 const &parentTransform,
                         glm::mat4 *pArmatureTransform) {
    foeNodeAnimationChannel const *pAnimChannel{nullptr};
    for (size_t channel = 0; channel < animationChannels.size(); ++channel) {
        if (animationChannels[channel].nodeName == pNode->name) {
            pAnimChannel = &animationChannels[channel];
        }
    }

    if (pAnimChannel == nullptr ||
        (pAnimChannel->positionKeys.empty() && pAnimChannel->rotationKeys.empty() &&
         pAnimChannel->scalingKeys.empty())) {
        *pArmatureTransform = pNode->transformMatrix;
    } else {
        glm::vec3 posVec = interpolatePosition(animationTick, pAnimChannel);
        glm::mat4 posMat = glm::translate(glm::mat4(1.f), posVec);

        glm::quat rotQuat = interpolateRotation(animationTick, pAnimChannel);
        glm::mat4 rotMat = glm::mat4_cast(rotQuat);

        glm::vec3 scaleVec = interpolateScaling(animationTick, pAnimChannel);
        glm::mat4 scaleMat = glm::scale(glm::mat4(1.f), scaleVec);

        *pArmatureTransform = parentTransform * posMat * rotMat * scaleMat;
    }

    for (size_t child = 0; child < pNode->numChildren; ++child) {
        size_t const offset = pNode->childrenOffset + child;

        animateArmatureNode(pNode + offset, animationChannels, animationTick, *pArmatureTransform,
                            pArmatureTransform + offset);
    }
}

} // namespace

auto foeArmatureSystem::initialize(foeResourcePool armaturePool,
                                   foeArmatureStatePool *pArmatureStatePool) -> std::error_code {
    if (armaturePool == nullptr) {
        return FOE_BRINGUP_ERROR_NO_ARMATURE_POOL_PROVIDED;
    }
    if (pArmatureStatePool == nullptr) {
        return FOE_BRINGUP_ERROR_NO_ARMATURE_STATE_POOL_PROVIDED;
    }

    mArmaturePool = armaturePool;
    mpArmatureStatePool = pArmatureStatePool;

    return FOE_BRINGUP_SUCCESS;
}

void foeArmatureSystem::deinitialize() {
    mpArmatureStatePool = nullptr;
    mArmaturePool = nullptr;
}

bool foeArmatureSystem::initialized() const noexcept { return mArmaturePool != nullptr; }

void foeArmatureSystem::process(float timePassed) {
    auto *pArmatureState = mpArmatureStatePool->begin<1>();
    auto const *pEndArmatureState = mpArmatureStatePool->end<1>();

    for (; pArmatureState != pEndArmatureState; ++pArmatureState) {
        // Add the time that has passed to the armature/animation
        pArmatureState->time += timePassed;

        // If there's no valid armature associated with this data, skip it
        if (pArmatureState->armatureID == FOE_INVALID_ID)
            continue;

        foeResource armature = foeResourcePoolFind(mArmaturePool, pArmatureState->armatureID);
        // If the armature we're trying to use isn't here, skip this entry
        if (armature == FOE_NULL_HANDLE) {
            std::abort();
        }

        if (foeResourceGetState(armature) != foeResourceLoadState::Loaded) {
            if (!foeResourceGetIsLoading(armature))
                foeResourceLoad(armature, false);

            continue;
        }

        foeArmature const *pArmature = (foeArmature const *)foeResourceGetData(armature);

        // If the animation index isn't on the given armature, then just set the default armature
        // values
        pArmatureState->armatureState.resize(pArmature->armature.size());
        if (pArmature->animations.size() <= pArmatureState->animationID) {
            // The original armature matrices
            originalArmatureNode(&pArmature->armature[0], glm::mat4{1.f},
                                 &pArmatureState->armatureState[0]);

            // Not applying animations, so continue to next entity
            continue;
        }

        foeAnimation const &animation = pArmature->animations[pArmatureState->animationID];

        auto const animationDuration = animation.duration / animation.ticksPerSecond;
        auto animationTime = pArmatureState->time;
        while (animationTime > animationDuration) {
            animationTime -= animationDuration;
        }

        animationTime *= animation.ticksPerSecond;

        animateArmatureNode(&pArmature->armature[0], animation.nodeChannels, animationTime,
                            glm::mat4{1.f}, &pArmatureState->armatureState[0]);
    }
}