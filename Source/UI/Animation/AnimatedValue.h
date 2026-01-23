/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#pragma once
#include <cmath>

namespace BlindCard {

/**
 * Easing types for animated transitions.
 */
enum class EasingType {
    Linear,      ///< Constant speed, no acceleration
    EaseOut,     ///< Fast start, slow end (cubic)
    EaseInOut    ///< Slow start and end, fast middle (cubic)
};

/**
 * AnimatedValue provides smooth value interpolation with configurable easing.
 *
 * Used for:
 * - Card flip animations (0.0 to 1.0 over 400ms)
 * - Hover effects (button glow intensity)
 * - Selection transitions
 * - Pulse animations
 *
 * Usage:
 *   AnimatedValue flipProgress(0.0f);
 *   flipProgress.setTarget(1.0f, 400.0f, EasingType::EaseOut);
 *
 *   // In update loop:
 *   flipProgress.update(deltaTimeMs);
 *   float progress = flipProgress.getValue();
 */
class AnimatedValue {
public:
    /**
     * Construct an AnimatedValue with an initial value.
     * @param initialValue Starting value (default 0.0f)
     */
    explicit AnimatedValue(float initialValue = 0.0f)
        : current(initialValue)
        , target(initialValue)
        , start(initialValue)
    {
    }

    /**
     * Set a new target value to animate towards.
     * Animation only starts if the new target differs from current target
     * by more than the threshold (0.001f).
     *
     * @param newTarget The value to animate towards
     * @param durationMs Duration of the animation in milliseconds
     * @param easing Easing function to use (default: EaseOut)
     */
    void setTarget(float newTarget, float durationMs, EasingType easing = EasingType::EaseOut)
    {
        // Only start animation if target actually changes (threshold 0.001f)
        if (std::abs(newTarget - target) < 0.001f)
            return;

        start = current;
        target = newTarget;
        duration = durationMs > 0.0f ? durationMs : 1.0f;
        elapsed = 0.0f;
        easingType = easing;
    }

    /**
     * Immediately set the value without animation.
     * Cancels any in-progress animation.
     *
     * @param value The value to set immediately
     */
    void setImmediate(float value)
    {
        current = value;
        target = value;
        start = value;
        elapsed = duration; // Mark as complete
    }

    /**
     * Update the animation by advancing time.
     * Call this each frame with the time delta.
     *
     * @param deltaMs Time elapsed since last update in milliseconds
     */
    void update(float deltaMs)
    {
        if (isComplete())
            return;

        elapsed += deltaMs;

        if (elapsed >= duration) {
            // Animation complete - snap to target
            current = target;
            elapsed = duration;
        } else {
            // Calculate normalized time (0.0 to 1.0)
            float t = elapsed / duration;

            // Apply easing function
            float easedT = applyEasing(t);

            // Interpolate between start and target
            current = start + (target - start) * easedT;
        }
    }

    /**
     * Check if the animation has completed.
     * @return true if animation is complete or no animation is active
     */
    bool isComplete() const
    {
        return elapsed >= duration;
    }

    /**
     * Get the current interpolated value.
     * @return The current animated value
     */
    float getValue() const
    {
        return current;
    }

    /**
     * Get the target value being animated towards.
     * @return The target value
     */
    float getTarget() const
    {
        return target;
    }

private:
    /**
     * Apply the selected easing function to a normalized time value.
     *
     * @param t Normalized time (0.0 to 1.0)
     * @return Eased value (0.0 to 1.0)
     */
    float applyEasing(float t) const
    {
        switch (easingType) {
            case EasingType::Linear:
                return t;

            case EasingType::EaseOut:
                // Cubic ease-out: fast start, slow end
                // Formula: 1.0 - (1.0 - t)^3
                return 1.0f - std::pow(1.0f - t, 3.0f);

            case EasingType::EaseInOut:
                // Two-phase cubic: slow start and end, fast middle
                // First half: accelerate (4 * t^3)
                // Second half: decelerate (1 - (-2t + 2)^3 / 2)
                if (t < 0.5f) {
                    return 4.0f * t * t * t;
                } else {
                    return 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
                }

            default:
                return t;
        }
    }

    float current = 0.0f;                          ///< Current interpolated value
    float target = 0.0f;                           ///< Target value to animate towards
    float start = 0.0f;                            ///< Starting value of current animation
    float duration = 1.0f;                         ///< Duration of animation in milliseconds
    float elapsed = 0.0f;                          ///< Time elapsed since animation started
    EasingType easingType = EasingType::EaseOut;   ///< Current easing function
};

} // namespace BlindCard
