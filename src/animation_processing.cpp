#include "animation_processing.hpp"

#include <foe/resource/armature.hpp>

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

void processArmatureStates(std::map<foeId, foeArmatureState> *pArmatureStates,
                           foeArmaturePool *pArmaturePool) {
    for (auto it = pArmatureStates->begin(); it != pArmatureStates->end(); ++it) {
        foeArmatureState &armatureState = it->second;

        // If there's no valid armature associated with this data, skip it
        if (armatureState.armatureID == FOE_INVALID_ID)
            continue;

        foeArmature *pArmature = pArmaturePool->find(armatureState.armatureID);
        // If the armature we're trying to use isn't here, skip this entry
        if (pArmature == nullptr || pArmature->getLoadState() != foeResourceLoadState::Loaded)
            continue;

        // If the animation index isn't on the given armature, then just set the default armature
        // values
        armatureState.armatureState.resize(pArmature->data.armature.size());
        if (pArmature->data.animations.size() <= armatureState.animationID) {
            // The original armature matrices
            originalArmatureNode(&pArmature->data.armature[0], glm::mat4{1.f},
                                 &armatureState.armatureState[0]);

            // Not applying animations, so continue to next entity
            continue;
        }

        foeAnimation &animation = pArmature->data.animations[armatureState.animationID];

        auto const animationDuration = animation.duration / animation.ticksPerSecond;
        auto animationTime = armatureState.time;
        while (animationTime > animationDuration) {
            animationTime -= animationDuration;
        }

        glm::mat4 transform{1.f};
        animationTime *= animation.ticksPerSecond;

        animateArmatureNode(&pArmature->data.armature[0], animation.nodeChannels, animationTime,
                            glm::mat4{1.f}, &armatureState.armatureState[0]);
    }
}