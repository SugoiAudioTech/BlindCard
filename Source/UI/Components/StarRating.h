/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    StarRating.h
    Created: 2026-01-19
    Author:  BlindCard

    5-star rating component with click and fill animation.
    Used on PokerCard during Stars mode in BlindTesting phase.

    Size: Each star ~20px, total ~120px wide

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include <array>
#include "../Theme/ThemeManager.h"
#include "../Animation/AnimatedValue.h"

namespace BlindCard
{

//==============================================================================
/**
 * StarRating displays a 5-star rating system with interactive click and
 * hover preview functionality.
 *
 * Visual design:
 * - Empty star: ☆ (U+2606) in gray (#4A4A4A dark / #D4D4D4 light)
 * - Filled star: ★ (U+2605) in gold (#FFD700 dark / #D4AF37 light)
 *
 * Animations:
 * - Click animation: clicked star scales 1.0 -> 1.3 -> 1.0 (200ms)
 * - Fill animation: stars fill left-to-right with 50ms delay each
 *
 * Usage:
 *   StarRating rating;
 *   rating.setInteractive(true);
 *   rating.onRatingChanged = [this](int newRating) { handleRating(newRating); };
 *   rating.setRating(3); // Set to 3 stars
 */
class StarRating : public juce::Component,
                   public juce::ChangeListener
{
public:
    //==========================================================================
    /** Constructor */
    StarRating();

    /** Destructor */
    ~StarRating() override;

    //==========================================================================
    /** Callback triggered when the rating changes via user interaction */
    std::function<void(int)> onRatingChanged;

    //==========================================================================
    /**
     * Sets the current rating.
     * @param rating Value 0-5, where 0 means no rating
     * @param animate If true, animate the fill transition (default: true)
     */
    void setRating(int rating, bool animate = true);

    /**
     * Returns the current rating (0-5).
     */
    int getRating() const { return currentRating; }

    /**
     * Sets whether the component is interactive.
     * When false, clicking has no effect and hover preview is disabled.
     * @param interactive true to enable interaction, false to disable
     */
    void setInteractive(bool interactive);

    /** Returns true if the component is interactive */
    bool isInteractive() const { return interactiveState; }

    //==========================================================================
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    void mouseMove(const juce::MouseEvent& event) override;
    void mouseDown(const juce::MouseEvent& event) override;

    // Timer callback for animation updates
    void updateAnimations();

    // ChangeListener override (for ThemeManager)
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    //==========================================================================
    /** Standard dimensions */
    static constexpr int kStarCount = 5;
    static constexpr int kStarSize = 20;
    static constexpr int kStarSpacing = 4;
    static constexpr int kBgPadding = 8;  // Padding around stars for background
    static constexpr int kTotalWidth = kStarCount * kStarSize + (kStarCount - 1) * kStarSpacing + kBgPadding * 2;
    static constexpr int kTotalHeight = kStarSize + kBgPadding;

private:
    //==========================================================================
    // State
    int currentRating = 0;          // Current rating (0-5)
    int hoverRating = 0;            // Rating preview on hover (0 = no hover)
    bool interactiveState = true;   // Whether clicks are enabled
    bool isHovered = false;         // Whether mouse is over component

    //==========================================================================
    // Animation values
    static constexpr int kMaxStars = 5;

    // Fill progress for each star (0.0 = empty, 1.0 = filled)
    std::array<AnimatedValue, kMaxStars> fillProgress;

    // Scale for click animation (1.0 = normal, 1.3 = max)
    std::array<AnimatedValue, kMaxStars> scaleProgress;

    // Animation timing constants
    static constexpr float kClickAnimationDurationMs = 200.0f;
    static constexpr float kFillAnimationDurationMs = 150.0f;
    static constexpr float kFillDelayMs = 50.0f;
    static constexpr float kClickScaleMax = 1.3f;

    // Track which star was clicked for scale animation
    int clickedStarIndex = -1;
    juce::int64 clickAnimationStartTime = 0;

    //==========================================================================
    // Timer for animations
    class AnimationTimer : public juce::Timer
    {
    public:
        AnimationTimer(StarRating& owner) : component(owner) {}
        void timerCallback() override { component.updateAnimations(); }
    private:
        StarRating& component;
    };
    std::unique_ptr<AnimationTimer> animationTimer;
    juce::int64 lastUpdateTime = 0;

    // Pending fill animation state
    int pendingFillRating = -1;
    juce::int64 fillAnimationStartTime = 0;

    //==========================================================================
    // Drawing helpers
    void drawStar(juce::Graphics& g, int index, juce::Rectangle<float> bounds,
                  bool filled, float fillAmount, float scale);

    // Returns the star index (0-4) at the given x position, or -1 if none
    int getStarIndexAtPosition(int x) const;

    // Returns the rating (1-5) for a star index (0-4)
    int ratingFromStarIndex(int index) const { return index + 1; }

    // Get star bounds for a given index
    juce::Rectangle<float> getStarBounds(int index) const;

    // Utility
    void startAnimationTimer();
    void stopAnimationTimerIfIdle();
    void triggerFillAnimation(int targetRating);
    void triggerClickAnimation(int starIndex);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StarRating)
};

} // namespace BlindCard
