/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    QuestionBanner.h
    Created: 2026-01-19
    Author:  BlindCard

    Banner component for Q&A mode displaying "Which card is [Plugin Name]?"
    with progress indicator and correct/wrong feedback.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <string>
#include "../Theme/ThemeManager.h"
#include "../Animation/AnimatedValue.h"

namespace BlindCard
{

//==============================================================================
/**
 * Feedback state for Q&A answers.
 */
enum class QAFeedback
{
    None,       // No feedback shown (question state)
    Correct,    // Green glow, checkmark
    Wrong       // Red glow, X mark
};

//==============================================================================
/**
 * QuestionBanner displays the Q&A mode question and feedback.
 *
 * Layout:
 * +----------------------------------------------------------+
 * |  "Which card is [Plugin Name]?"        (1/4 remaining)   |
 * +----------------------------------------------------------+
 *
 * Visual Design:
 * - Surface background with rounded corners
 * - Question text in primary text color
 * - Progress indicator (e.g., "1/4 remaining")
 * - Feedback states: success (green), error (red)
 *
 * Animation:
 * - Pulse glow on feedback
 * - Fade transitions between questions
 *
 * Usage:
 *   QuestionBanner banner;
 *   banner.setQuestion("Plugin A - Compressor");
 *   banner.setProgress(1, 4);
 *   banner.showFeedback(QAFeedback::Correct);
 */
class QuestionBanner : public juce::Component,
                       public juce::ChangeListener
{
public:
    //==========================================================================
    /** Constructor */
    QuestionBanner();

    /** Destructor */
    ~QuestionBanner() override;

    //==========================================================================
    /**
     * Sets the plugin name for the question.
     * Displays "Which card is [pluginName]?"
     * @param pluginName The plugin name to ask about
     */
    void setQuestion(const std::string& pluginName);

    /** Returns the current plugin name */
    const std::string& getPluginName() const { return currentPluginName; }

    /**
     * Sets the progress indicator.
     * @param current Current question number (1-based)
     * @param total Total number of questions
     */
    void setProgress(int current, int total);

    /**
     * Shows feedback for an answer.
     * @param feedback None, Correct, or Wrong
     */
    void showFeedback(QAFeedback feedback);

    /** Returns the current feedback state */
    QAFeedback getFeedback() const { return currentFeedback; }

    /**
     * Clears the feedback (back to question state).
     */
    void clearFeedback();

    /**
     * Sets the countdown value (3, 2, 1, 0).
     * When > 0, displays countdown overlay.
     * @param countdown Countdown value (0 = no countdown)
     */
    void setCountdown(int countdown);

    /** Returns the current countdown value */
    int getCountdown() const { return countdownValue; }

    /**
     * Sets the correct answer track name to display during reveal.
     * @param trackName The correct answer track name
     */
    void setCorrectAnswer(const std::string& trackName);

    /**
     * Shows completion state with score and reset prompt.
     * @param correct Number of correct answers
     * @param total Total number of questions
     */
    void showCompletion(int correct, int total);

    /**
     * Sets visibility with animation.
     * @param visible true to show, false to hide
     */
    void setVisibleAnimated(bool visible);

    //==========================================================================
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;

    // Timer callback for animation updates
    void updateAnimations();

    // ChangeListener override (for ThemeManager)
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    //==========================================================================
    /** Dimensions */
    static constexpr int kPreferredHeight = 48;
    static constexpr int kCornerRadius = 12;

private:
    //==========================================================================
    // State
    std::string currentPluginName;
    std::string correctAnswerName;         // 正確答案的軌道名
    int currentQuestion = 0;
    int totalQuestions = 0;
    int countdownValue = 0;                // 倒數值 (3, 2, 1, 0)
    QAFeedback currentFeedback = QAFeedback::None;

    // Completion state
    bool isComplete = false;
    int correctCount = 0;
    int totalCount = 0;

    //==========================================================================
    // Animation values
    AnimatedValue feedbackGlow;        // Glow animation for feedback
    AnimatedValue feedbackPulse;       // Pulse effect
    AnimatedValue visibilityAlpha;     // Fade in/out

    // Animation constants
    static constexpr float kFeedbackDurationMs = 300.0f;
    static constexpr float kPulseDurationMs = 800.0f;
    static constexpr float kFadeDurationMs = 200.0f;

    //==========================================================================
    // Timer for animations
    class AnimationTimer : public juce::Timer
    {
    public:
        AnimationTimer(QuestionBanner& owner) : banner(owner) {}
        void timerCallback() override { banner.updateAnimations(); }
    private:
        QuestionBanner& banner;
    };
    std::unique_ptr<AnimationTimer> animationTimer;
    juce::int64 lastUpdateTime = 0;
    juce::int64 pulseStartTime = 0;

    //==========================================================================
    // Drawing helpers
    void drawBackground(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawQuestion(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawProgress(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawFeedbackGlow(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawFeedbackIcon(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawCountdown(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawCorrectAnswer(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawCompletion(juce::Graphics& g, juce::Rectangle<float> bounds);

    // Animation utilities
    void startAnimationTimer();
    void stopAnimationTimerIfIdle();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuestionBanner)
};

} // namespace BlindCard
