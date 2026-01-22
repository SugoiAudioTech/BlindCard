/*
  ==============================================================================

    QuestionBanner.cpp
    Created: 2026-01-19
    Author:  BlindCard

  ==============================================================================
*/

#include "QuestionBanner.h"

namespace BlindCard
{

//==============================================================================
QuestionBanner::QuestionBanner()
    : visibilityAlpha(1.0f)
{
    ThemeManager::getInstance().addChangeListener(this);

    animationTimer = std::make_unique<AnimationTimer>(*this);
}

QuestionBanner::~QuestionBanner()
{
    ThemeManager::getInstance().removeChangeListener(this);

    if (animationTimer)
        animationTimer->stopTimer();
}

//==============================================================================
void QuestionBanner::setQuestion(const std::string& pluginName)
{
    currentPluginName = pluginName;
    isComplete = false;  // Reset completion state when new question is set
    clearFeedback();
    repaint();
}

void QuestionBanner::setProgress(int current, int total)
{
    currentQuestion = current;
    totalQuestions = total;
    repaint();
}

void QuestionBanner::showFeedback(QAFeedback feedback)
{
    currentFeedback = feedback;

    if (feedback != QAFeedback::None)
    {
        feedbackGlow.setTarget(1.0f, kFeedbackDurationMs, EasingType::EaseOut);
        pulseStartTime = juce::Time::currentTimeMillis();
        startAnimationTimer();
    }

    repaint();
}

void QuestionBanner::clearFeedback()
{
    currentFeedback = QAFeedback::None;
    feedbackGlow.setImmediate(0.0f);
    countdownValue = 0;
    correctAnswerName.clear();
    repaint();
}

void QuestionBanner::setCountdown(int countdown)
{
    countdownValue = countdown;
    repaint();
}

void QuestionBanner::setCorrectAnswer(const std::string& trackName)
{
    correctAnswerName = trackName;
    repaint();
}

void QuestionBanner::showCompletion(int correct, int total)
{
    isComplete = true;
    correctCount = correct;
    totalCount = total;
    currentFeedback = QAFeedback::None;
    countdownValue = 0;
    repaint();
}

void QuestionBanner::setVisibleAnimated(bool visible)
{
    float targetAlpha = visible ? 1.0f : 0.0f;
    visibilityAlpha.setTarget(targetAlpha, kFadeDurationMs, EasingType::EaseOut);
    startAnimationTimer();
}

//==============================================================================
void QuestionBanner::changeListenerCallback(juce::ChangeBroadcaster*)
{
    repaint();
}

//==============================================================================
void QuestionBanner::updateAnimations()
{
    auto now = juce::Time::currentTimeMillis();
    float deltaMs = static_cast<float>(now - lastUpdateTime);
    lastUpdateTime = now;

    feedbackGlow.update(deltaMs);
    visibilityAlpha.update(deltaMs);

    // Update pulse phase
    if (currentFeedback != QAFeedback::None)
    {
        float elapsed = static_cast<float>(now - pulseStartTime);
        float phase = std::fmod(elapsed, kPulseDurationMs) / kPulseDurationMs;
        feedbackPulse.setImmediate(0.5f + 0.5f * std::sin(phase * juce::MathConstants<float>::twoPi));
    }

    repaint();
    stopAnimationTimerIfIdle();
}

void QuestionBanner::startAnimationTimer()
{
    if (!animationTimer->isTimerRunning())
    {
        lastUpdateTime = juce::Time::currentTimeMillis();
        animationTimer->startTimerHz(60);
    }
}

void QuestionBanner::stopAnimationTimerIfIdle()
{
    bool hasActiveAnimation = !feedbackGlow.isComplete() ||
                              !visibilityAlpha.isComplete() ||
                              currentFeedback != QAFeedback::None;

    if (!hasActiveAnimation)
    {
        animationTimer->stopTimer();
    }
}

//==============================================================================
void QuestionBanner::paint(juce::Graphics& g)
{
    float alpha = visibilityAlpha.getValue();
    if (alpha < 0.01f)
        return;

    auto bounds = getLocalBounds().toFloat().reduced(4.0f);

    // Apply alpha for visibility animation
    g.saveState();
    g.setOpacity(alpha);

    // Draw feedback glow first (behind background)
    if (currentFeedback != QAFeedback::None)
    {
        drawFeedbackGlow(g, bounds);
    }

    // Draw background
    drawBackground(g, bounds);

    // Draw content
    auto contentBounds = bounds.reduced(16.0f, 8.0f);

    // Handle completion state
    if (isComplete)
    {
        drawCompletion(g, contentBounds);
    }
    else
    {
        // Draw feedback icon on the left if feedback is shown
        if (currentFeedback != QAFeedback::None)
        {
            auto iconArea = contentBounds.removeFromLeft(32.0f);
            drawFeedbackIcon(g, iconArea);
            contentBounds.removeFromLeft(8.0f);
        }

        // If showing countdown, display correct answer and countdown
        if (countdownValue > 0)
        {
            // Draw countdown on the right
            auto countdownArea = contentBounds.removeFromRight(60.0f);
            drawCountdown(g, countdownArea);

            // Draw correct answer in the center
            drawCorrectAnswer(g, contentBounds);
        }
        else
        {
            // Draw progress on the right
            auto progressArea = contentBounds.removeFromRight(100.0f);
            drawProgress(g, progressArea);

            // Draw question in the remaining space
            drawQuestion(g, contentBounds);
        }
    }

    g.restoreState();
}

void QuestionBanner::drawBackground(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();

    // Background
    g.setColour(tm.getColour(ColourId::Surface));
    g.fillRoundedRectangle(bounds, static_cast<float>(kCornerRadius));

    // Border
    g.setColour(tm.getColour(ColourId::SurfaceAlt));
    g.drawRoundedRectangle(bounds, static_cast<float>(kCornerRadius), 1.0f);
}

void QuestionBanner::drawQuestion(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();

    // Question text
    juce::String questionText = "Which card is " + juce::String(currentPluginName) + "?";

    g.setColour(tm.getColour(ColourId::TextPrimary));
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText(questionText, bounds, juce::Justification::centredLeft);
}

void QuestionBanner::drawProgress(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();

    if (totalQuestions > 0)
    {
        int remaining = totalQuestions - currentQuestion + 1;
        juce::String progressText = juce::String(remaining) + "/" +
                                    juce::String(totalQuestions) + " remaining";

        g.setColour(tm.getColour(ColourId::TextSecondary));
        g.setFont(juce::Font(12.0f));
        g.drawText(progressText, bounds, juce::Justification::centredRight);
    }
}

void QuestionBanner::drawFeedbackGlow(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();
    float glowAmount = feedbackGlow.getValue() * feedbackPulse.getValue();

    juce::Colour glowColor;
    if (currentFeedback == QAFeedback::Correct)
    {
        glowColor = tm.getColour(ColourId::Success);
    }
    else
    {
        glowColor = tm.getColour(ColourId::Error);
    }

    // Draw multiple glow layers
    for (int i = 3; i >= 1; --i)
    {
        float expand = static_cast<float>(i * 4) * glowAmount;
        float layerAlpha = 0.15f * glowAmount / static_cast<float>(i);
        g.setColour(glowColor.withAlpha(layerAlpha));
        g.fillRoundedRectangle(bounds.expanded(expand),
                               static_cast<float>(kCornerRadius) + expand);
    }
}

void QuestionBanner::drawFeedbackIcon(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();

    // Icon background circle
    auto iconBounds = bounds.withSizeKeepingCentre(28.0f, 28.0f);

    juce::Colour iconColor;
    juce::String iconText;

    if (currentFeedback == QAFeedback::Correct)
    {
        iconColor = tm.getColour(ColourId::Success);
        iconText = juce::String::fromUTF8("✓");
    }
    else
    {
        iconColor = tm.getColour(ColourId::Error);
        iconText = juce::String::fromUTF8("✗");
    }

    // Circle background
    g.setColour(iconColor.withAlpha(0.2f));
    g.fillEllipse(iconBounds);

    // Circle border
    g.setColour(iconColor);
    g.drawEllipse(iconBounds, 2.0f);

    // Icon
    g.setColour(iconColor);
    g.setFont(juce::Font(18.0f, juce::Font::bold));
    g.drawText(iconText, iconBounds, juce::Justification::centred);
}

void QuestionBanner::drawCountdown(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();

    // Draw countdown number with large font
    juce::String countdownText = juce::String(countdownValue);

    // Use accent color for countdown
    g.setColour(tm.getColour(ColourId::Accent));
    g.setFont(juce::Font(24.0f, juce::Font::bold));
    g.drawText(countdownText, bounds, juce::Justification::centred);
}

void QuestionBanner::drawCorrectAnswer(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();

    // Show "Answer: [Track Name]" with emphasis
    juce::String answerText = "Answer: " + juce::String(correctAnswerName);

    // Use success/error color based on feedback
    juce::Colour textColor = tm.getColour(ColourId::TextPrimary);
    if (currentFeedback == QAFeedback::Correct)
        textColor = tm.getColour(ColourId::Success);
    else if (currentFeedback == QAFeedback::Wrong)
        textColor = tm.getColour(ColourId::Error);

    g.setColour(textColor);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText(answerText, bounds, juce::Justification::centredLeft);
}

void QuestionBanner::drawCompletion(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();

    // Calculate score percentage for color
    float scorePercent = totalCount > 0 ? static_cast<float>(correctCount) / static_cast<float>(totalCount) : 0.0f;

    // Choose color based on score
    juce::Colour scoreColor;
    if (scorePercent >= 0.8f)
        scoreColor = tm.getColour(ColourId::Success);  // Green for 80%+
    else if (scorePercent >= 0.5f)
        scoreColor = tm.getColour(ColourId::Accent);   // Accent for 50%+
    else
        scoreColor = tm.getColour(ColourId::Error);    // Red for below 50%

    // Draw score on the left
    juce::String scoreText = "Score: " + juce::String(correctCount) + "/" + juce::String(totalCount);
    g.setColour(scoreColor);
    g.setFont(juce::Font(18.0f, juce::Font::bold));

    auto scoreArea = bounds.removeFromLeft(bounds.getWidth() * 0.4f);
    g.drawText(scoreText, scoreArea, juce::Justification::centredLeft);

    // Draw reset prompt on the right
    juce::String resetText = "Press Reset to play again";
    g.setColour(tm.getColour(ColourId::TextSecondary));
    g.setFont(juce::Font(14.0f));
    g.drawText(resetText, bounds, juce::Justification::centredRight);
}

void QuestionBanner::resized()
{
    // Nothing specific - layout is done in paint()
}

} // namespace BlindCard
