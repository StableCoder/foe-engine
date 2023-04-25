// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_MODEL_ANIMATION_HPP
#define FOE_MODEL_ANIMATION_HPP

#include <foe/model/export.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <string>
#include <vector>

/// Time/Value pair for an animation position keyframe
struct foeAnimationPositionKey {
    double time;
    glm::vec3 value;
};

/// Time/Value pair for an animation rotation keyframe
struct foeAnimationRotationKey {
    double time;
    glm::quat value;
};

/// Time/Value pair for an animation scaling keyframe
struct foeAnimationScalingKey {
    double time;
    glm::vec3 value;
};

/// Set of animation keyframes for a particular armature node within an animation
struct foeNodeAnimationChannel {
    std::string nodeName;

    std::vector<foeAnimationPositionKey> positionKeys;
    std::vector<foeAnimationRotationKey> rotationKeys;
    std::vector<foeAnimationScalingKey> scalingKeys;
};

/// Stores all data related to an animnation
struct foeAnimation {
    std::string name;
    double duration;
    double ticksPerSecond;

    /// Node-based animation keyframes
    std::vector<foeNodeAnimationChannel> nodeChannels;
};

/** @brief Returns an interpolated position based on the given time and node animation channel
 * @param time Time to use for interpolation
 * @param pAnimationChannel Animation keyframe data
 * @return Interpolated position
 *
 * Calculated like so:
 * - If there are no keyframes, returns vec3(0.f).
 * - If the time is before the first keyframe, returns the first one.
 * - If the time is after the last keyframe, returns the last one.
 * - If it's on a keyframe, returns that.
 * - If between keyframes, returns an itnerpolation between the two.
 */
FOE_MODEL_EXPORT
glm::vec3 interpolatePosition(double time, foeNodeAnimationChannel const *pAnimationChannel);

/** @brief Returns an interpolated rotation based on the given time and node animation channel
 * @param time Time to use for interpolation
 * @param pAnimationChannel Animation keyframe data
 * @return Interpolated rotation
 *
 * Calculated like so:
 * - If there are no keyframes, returns quat(vec3(0.f)).
 * - If the time is before the first keyframe, returns the first one.
 * - If the time is after the last keyframe, returns the last one.
 * - If it's on a keyframe, returns that.
 * - If between keyframes, returns an itnerpolation between the two.
 */
FOE_MODEL_EXPORT
glm::quat interpolateRotation(double time, foeNodeAnimationChannel const *pAnimationChannel);

/** @brief Returns an interpolated scaling based on the given time and node animation channel
 * @param time Time to use for interpolation
 * @param pAnimationChannel Animation keyframe data
 * @return Interpolated scaling
 *
 * Calculated like so:
 * - If there are no keyframes, returns vec3(1.f).
 * - If the time is before the first keyframe, returns the first one.
 * - If the time is after the last keyframe, returns the last one.
 * - If it's on a keyframe, returns that.
 * - If between keyframes, returns an itnerpolation between the two.
 */
FOE_MODEL_EXPORT
glm::vec3 interpolateScaling(double time, foeNodeAnimationChannel const *pAnimationChannel);

#endif // FOE_MODEL_ANIMATION_HPP