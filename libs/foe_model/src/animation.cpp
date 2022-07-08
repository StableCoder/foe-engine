// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/model/animation.hpp>

glm::vec3 interpolatePosition(double time, foeNodeAnimationChannel const *pAnimationChannel) {
    // If there's no keys, return no position
    if (pAnimationChannel->positionKeys.empty()) {
        return glm::vec3(0.f);
    }

    // If there's only one key, OR the time is before the first key, return the first key
    if (pAnimationChannel->positionKeys.size() == 1 ||
        time < pAnimationChannel->positionKeys[0].time) {
        return pAnimationChannel->positionKeys[0].value;
    }

    size_t index = 0;
    for (size_t i = 1; i < pAnimationChannel->positionKeys.size(); ++i) {
        if (time < pAnimationChannel->positionKeys[i].time) {
            break;
        }
        index = i;
    }

    // If it's the last key, return it alone
    if (index == pAnimationChannel->positionKeys.size() - 1) {
        return pAnimationChannel->positionKeys[index].value;
    }

    // If here, we're interpolating between two keys
    size_t nextIndex = index + 1;
    double deltaTime = pAnimationChannel->positionKeys[nextIndex].time -
                       pAnimationChannel->positionKeys[index].time;

    // The percentage through between the keyframes we're at
    double factorTime = (time - pAnimationChannel->positionKeys[index].time) / deltaTime;

    glm::vec3 const &startPos = pAnimationChannel->positionKeys[index].value;
    glm::vec3 const &endPos = pAnimationChannel->positionKeys[nextIndex].value;

    return startPos + ((endPos - startPos) * static_cast<float>(factorTime));
}

glm::quat interpolateRotation(double time, foeNodeAnimationChannel const *pAnimationChannel) {
    // If there's no keys, return no rotation
    if (pAnimationChannel->rotationKeys.empty()) {
        return glm::vec3(0.f);
    }

    // If there's only one key, OR the time is before the first key, return the first key
    if (pAnimationChannel->rotationKeys.size() == 1 ||
        time < pAnimationChannel->rotationKeys[0].time) {
        return pAnimationChannel->rotationKeys[0].value;
    }

    size_t index = 0;
    for (size_t i = 1; i < pAnimationChannel->rotationKeys.size(); ++i) {
        if (time < pAnimationChannel->rotationKeys[i].time) {
            break;
        }
        index = i;
    }

    // If it's the last key, return it alone
    if (index == pAnimationChannel->rotationKeys.size() - 1) {
        return pAnimationChannel->rotationKeys[index].value;
    }

    // If here, we're interpolating between two keys
    size_t nextIndex = index + 1;
    double deltaTime = pAnimationChannel->rotationKeys[nextIndex].time -
                       pAnimationChannel->rotationKeys[index].time;

    // The percentage through between the keyframes we're at
    double factorTime = (time - pAnimationChannel->rotationKeys[index].time) / deltaTime;

    glm::quat const &startPos = pAnimationChannel->rotationKeys[index].value;
    glm::quat const &endPos = pAnimationChannel->rotationKeys[nextIndex].value;

    return startPos + ((endPos - startPos) * static_cast<float>(factorTime));
}

glm::vec3 interpolateScaling(double time, foeNodeAnimationChannel const *pAnimationChannel) {
    // If there's no keys, return no scaling
    if (pAnimationChannel->scalingKeys.empty()) {
        return glm::vec3(1.f);
    }

    // If there's only one key, OR the time is before the first key, return the first key
    if (pAnimationChannel->scalingKeys.size() == 1 ||
        time < pAnimationChannel->scalingKeys[0].time) {
        return pAnimationChannel->scalingKeys[0].value;
    }

    size_t index = 0;
    for (size_t i = 1; i < pAnimationChannel->scalingKeys.size(); ++i) {
        if (time < pAnimationChannel->scalingKeys[i].time) {
            break;
        }
        index = i;
    }

    // If it's the last key, return it alone
    if (index == pAnimationChannel->scalingKeys.size() - 1) {
        return pAnimationChannel->scalingKeys[index].value;
    }

    // If here, we're interpolating between two keys
    size_t nextIndex = index + 1;
    double deltaTime =
        pAnimationChannel->scalingKeys[nextIndex].time - pAnimationChannel->scalingKeys[index].time;

    // The percentage through between the keyframes we're at
    double factorTime = (time - pAnimationChannel->scalingKeys[index].time) / deltaTime;

    glm::vec3 const &startPos = pAnimationChannel->scalingKeys[index].value;
    glm::vec3 const &endPos = pAnimationChannel->scalingKeys[nextIndex].value;

    return startPos + ((endPos - startPos) * static_cast<float>(factorTime));
}