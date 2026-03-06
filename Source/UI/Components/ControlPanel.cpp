/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    ControlPanel.cpp
    Created: 2026-01-19
    Author:  BlindCard

  ==============================================================================
*/

#include "ControlPanel.h"
#include "../Theme/FontManager.h"

namespace BlindCard
{

//==============================================================================
// Helper to check if current language needs CJK-compatible font
namespace
{
    bool isCJKLanguage()
    {
        auto lang = LocalizationManager::getInstance().getCurrentLanguage();
        return lang == Language::TraditionalChinese ||
               lang == Language::SimplifiedChinese ||
               lang == Language::Japanese ||
               lang == Language::Korean;
    }
}

//==============================================================================
// Custom slider look and feel
void ControlPanel::CustomSliderLookAndFeel::drawLinearSlider(
    juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
    const juce::Slider::SliderStyle, juce::Slider& slider)
{
    auto& tm = ThemeManager::getInstance();

    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    auto trackHeight = 4.0f;
    auto trackBounds = bounds.withSizeKeepingCentre(bounds.getWidth(), trackHeight);

    // Track background
    g.setColour(tm.getColour(ColourId::SliderTrack));
    g.fillRoundedRectangle(trackBounds, trackHeight / 2.0f);

    // Filled portion
    auto filledWidth = sliderPos - static_cast<float>(x);
    auto filledBounds = trackBounds.withWidth(filledWidth);
    g.setColour(tm.getColour(ColourId::Primary));
    g.fillRoundedRectangle(filledBounds, trackHeight / 2.0f);

    // Thumb
    auto thumbSize = 14.0f;
    auto thumbBounds = juce::Rectangle<float>(thumbSize, thumbSize)
                           .withCentre({ sliderPos, bounds.getCentreY() });

    // Thumb shadow (for depth)
    g.setColour(juce::Colours::black.withAlpha(0.15f));
    g.fillEllipse(thumbBounds.translated(0.0f, 1.0f));

    // Thumb body
    g.setColour(juce::Colours::white);
    g.fillEllipse(thumbBounds);

    // Thumb border (for visibility in light mode)
    bool isDark = tm.isDark();
    g.setColour(isDark ? juce::Colours::white.withAlpha(0.3f) : juce::Colours::black.withAlpha(0.25f));
    g.drawEllipse(thumbBounds, 1.0f);
}

//==============================================================================
// Custom toggle look and feel
void ControlPanel::CustomToggleLookAndFeel::drawToggleButton(
    juce::Graphics& g, juce::ToggleButton& button,
    bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto& tm = ThemeManager::getInstance();
    auto bounds = button.getLocalBounds().toFloat();

    // Toggle switch dimensions
    float switchWidth = 44.0f;
    float switchHeight = 24.0f;
    float thumbSize = 18.0f;

    auto switchBounds = bounds.withSizeKeepingCentre(switchWidth, switchHeight);

    bool isOn = button.getToggleState();

    // Switch background
    auto bgColour = isOn ? tm.getColour(ColourId::Primary) : tm.getColour(ColourId::SliderTrack);
    g.setColour(bgColour);
    g.fillRoundedRectangle(switchBounds, switchHeight / 2.0f);

    // Border
    g.setColour(isOn ? tm.getColour(ColourId::Primary).darker(0.2f) : juce::Colours::black.withAlpha(0.3f));
    g.drawRoundedRectangle(switchBounds, switchHeight / 2.0f, 1.0f);

    // Thumb position
    float thumbX = isOn
        ? switchBounds.getRight() - thumbSize - 3.0f
        : switchBounds.getX() + 3.0f;

    auto thumbBounds = juce::Rectangle<float>(thumbX, switchBounds.getCentreY() - thumbSize / 2.0f,
                                               thumbSize, thumbSize);

    // Thumb
    g.setColour(juce::Colours::white);
    g.fillEllipse(thumbBounds);
}

//==============================================================================
ControlPanel::ControlPanel()
{
    ThemeManager::getInstance().addChangeListener(this);
    LocalizationManager::getInstance().addListener(this);

    // Create custom look and feels
    sliderLookAndFeel = std::make_unique<CustomSliderLookAndFeel>();
    toggleLookAndFeel = std::make_unique<CustomToggleLookAndFeel>();

    // Create rounds slider
    roundsSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal,
                                                   juce::Slider::NoTextBox);
    roundsSlider->setRange(1.0, 8.0, 1.0);
    roundsSlider->setValue(3.0);
    roundsSlider->setLookAndFeel(sliderLookAndFeel.get());
    roundsSlider->onValueChange = [this]()
    {
        if (onRoundsChanged)
            onRoundsChanged(static_cast<int>(roundsSlider->getValue()));
        repaint();  // Update round counter display
    };
    addAndMakeVisible(*roundsSlider);

    // Create Q&A questions slider (only visible in Q&A mode)
    qaQuestionsSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal,
                                                        juce::Slider::NoTextBox);
    qaQuestionsSlider->setRange(1.0, 8.0, 1.0);
    qaQuestionsSlider->setValue(5.0);  // 預設 5 題
    qaQuestionsSlider->setLookAndFeel(sliderLookAndFeel.get());
    qaQuestionsSlider->onValueChange = [this]()
    {
        if (onQAQuestionsChanged)
            onQAQuestionsChanged(static_cast<int>(qaQuestionsSlider->getValue()));
        repaint();
    };
    addAndMakeVisible(*qaQuestionsSlider);
    qaQuestionsSlider->setVisible(false);  // Hidden by default

    // Create auto gain toggle (no text - we draw it ourselves)
    autoGainToggle = std::make_unique<juce::ToggleButton>();
    autoGainToggle->setToggleState(true, juce::dontSendNotification);
    autoGainToggle->setLookAndFeel(toggleLookAndFeel.get());
    autoGainToggle->onClick = [this]()
    {
        autoGainEnabled = autoGainToggle->getToggleState();
        if (onAutoGainChanged)
            onAutoGainChanged(autoGainEnabled);
    };
    addAndMakeVisible(*autoGainToggle);

    // Create action buttons with localized labels (only REVEAL and RESET visible initially)
    shuffleButton = std::make_unique<ChipButton>(LOCALIZE(ButtonShuffle), ChipVariant::Red);
    shuffleButton->onClick = [this]()
    {
        if (onShuffleClicked)
            onShuffleClicked();
    };
    addAndMakeVisible(*shuffleButton);
    shuffleButton->setVisible(false);  // Hidden by default

    revealButton = std::make_unique<ChipButton>(LOCALIZE(ButtonReveal), ChipVariant::Blue);
    revealButton->onClick = [this]()
    {
        if (onRevealClicked)
            onRevealClicked();
    };
    addAndMakeVisible(*revealButton);

    resetButton = std::make_unique<ChipButton>(LOCALIZE(ButtonReset), ChipVariant::Black);
    resetButton->onClick = [this]()
    {
        if (onResetClicked)
            onResetClicked();
    };
    addAndMakeVisible(*resetButton);

    nextRoundButton = std::make_unique<ChipButton>(LOCALIZE(ButtonNextRound), ChipVariant::Gold);
    nextRoundButton->onClick = [this]()
    {
        if (onNextRoundClicked)
            onNextRoundClicked();
    };
    addAndMakeVisible(*nextRoundButton);
    nextRoundButton->setVisible(false);  // Hidden by default
}

ControlPanel::~ControlPanel()
{
    ThemeManager::getInstance().removeChangeListener(this);
    LocalizationManager::getInstance().removeListener(this);
    roundsSlider->setLookAndFeel(nullptr);
    if (qaQuestionsSlider)
        qaQuestionsSlider->setLookAndFeel(nullptr);
    autoGainToggle->setLookAndFeel(nullptr);
}

//==============================================================================
void ControlPanel::setTracks(const std::vector<std::string>& tracks)
{
    trackNames = tracks;
    repaint();
}

void ControlPanel::setRounds(int rounds)
{
    roundsSlider->setValue(static_cast<double>(rounds), juce::dontSendNotification);
    repaint();
}

int ControlPanel::getRounds() const
{
    return static_cast<int>(roundsSlider->getValue());
}

void ControlPanel::setStandaloneMode(bool standalone)
{
    isStandaloneModeActive = standalone;

    // Level-Match toggle is now always visible (works in both DAW and Standalone modes)
    autoGainToggle->setVisible(true);

    resized();
    repaint();
}

void ControlPanel::setAutoGain(bool enabled)
{
    autoGainEnabled = enabled;
    autoGainToggle->setToggleState(enabled, juce::dontSendNotification);
    repaint();
}

void ControlPanel::setCalibrationStatus(bool calibrating, bool calibrated, float timeRemaining)
{
    calibratingStatus = calibrating;
    calibratedStatus = calibrated;
    calibrationTimeRemaining = timeRemaining;
    repaint();
}

void ControlPanel::setQAQuestions(int count)
{
    if (qaQuestionsSlider)
        qaQuestionsSlider->setValue(static_cast<double>(count), juce::dontSendNotification);
    repaint();
}

int ControlPanel::getQAQuestions() const
{
    if (qaQuestionsSlider)
        return static_cast<int>(qaQuestionsSlider->getValue());
    return 5;
}

void ControlPanel::setQAMode(bool isQA)
{
    isQAModeActive = isQA;
    if (qaQuestionsSlider)
        qaQuestionsSlider->setVisible(isQA);
    roundsSlider->setVisible(!isQA);
    resized();
    repaint();
}

void ControlPanel::setCurrentRound(int round)
{
    currentRound = round;
    repaint();
}

//==============================================================================
void ControlPanel::setShuffleEnabled(bool enabled)
{
    shuffleButton->setEnabled(enabled);
    shuffleButton->setVisible(enabled);
    resized();
}

void ControlPanel::setRevealEnabled(bool enabled)
{
    revealButton->setEnabled(enabled);
}

void ControlPanel::setResetEnabled(bool enabled)
{
    resetButton->setEnabled(enabled);
}

void ControlPanel::setNextRoundEnabled(bool enabled)
{
    nextRoundButton->setEnabled(enabled);
    nextRoundButton->setVisible(enabled);
    resized();
}

//==============================================================================
void ControlPanel::changeListenerCallback(juce::ChangeBroadcaster*)
{
    repaint();
}

void ControlPanel::languageChanged()
{
    // Update button labels with localized text
    shuffleButton->setLabel(LOCALIZE(ButtonShuffle));
    revealButton->setLabel(LOCALIZE(ButtonReveal));
    resetButton->setLabel(LOCALIZE(ButtonReset));
    nextRoundButton->setLabel(LOCALIZE(ButtonNextRound));
    repaint();
}

//==============================================================================
void ControlPanel::paint(juce::Graphics& g)
{
    auto& tm = ThemeManager::getInstance();
    auto bounds = getLocalBounds().toFloat();

    // Panel background
    g.setColour(tm.getColour(ColourId::Surface));
    g.fillRoundedRectangle(bounds, 8.0f);

    // Content area
    auto contentBounds = bounds.reduced(20.0f, 16.0f);

    // Top info row
    auto infoRow = contentBounds.removeFromTop(32.0f);
    drawInfoRow(g, infoRow);

    contentBounds.removeFromTop(16.0f);  // Gap

    // Auto gain row (Level-Match works in both DAW and Standalone modes)
    auto autoGainRow = contentBounds.removeFromTop(40.0f);
    drawAutoGainRow(g, autoGainRow);
}

void ControlPanel::drawInfoRow(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();
    auto& fonts = FontManager::getInstance();

    // Reserve space for round counter on the right first (50px)
    auto counterBounds = bounds.removeFromRight(50.0f);

    // Left section: Current Tracks
    auto leftSection = bounds.removeFromLeft(bounds.getWidth() * 0.4f);

    // Music note icon
    g.setColour(tm.getColour(ColourId::Primary));
    g.setFont(fonts.getRegular(18.0f));
    auto iconBounds = leftSection.removeFromLeft(24.0f);
    g.drawText(juce::String::fromUTF8("♪"), iconBounds, juce::Justification::centred);

    leftSection.removeFromLeft(8.0f);  // Gap

    // "TRACKS" label - Casino style for English, CJK-compatible for Asian languages
    g.setColour(tm.getColour(ColourId::TextMuted));
    if (isCJKLanguage())
        g.setFont(fonts.getMedium(16.0f));
    else
        g.setFont(fonts.getBebasNeue(14.0f));
    auto labelBounds = leftSection.removeFromLeft(60.0f);
    g.drawText(LOCALIZE(LabelCards), labelBounds, juce::Justification::centredLeft);

    // Track count - Casino style: Bebas Neue
    int trackCount = static_cast<int>(trackNames.size());
    if (trackCount == 0) trackCount = 4;  // Default display

    g.setColour(tm.getColour(ColourId::TextPrimary));
    g.setFont(fonts.getBebasNeue(22.0f));
    auto countBounds = leftSection.removeFromLeft(30.0f);
    g.drawText(juce::String(trackCount), countBounds, juce::Justification::centredLeft);

    bounds.removeFromLeft(16.0f);  // Gap

    // Right section: Rounds (or Questions in Q&A mode)
    // "ROUNDS" / "QUESTIONS" label - Casino style for English, CJK-compatible for Asian languages
    g.setColour(tm.getColour(ColourId::TextMuted));
    if (isCJKLanguage())
        g.setFont(fonts.getMedium(16.0f));
    else
        g.setFont(fonts.getBebasNeue(14.0f));
    juce::String labelText = isQAModeActive ? LOCALIZE(LabelQuestions) : LOCALIZE(LabelRounds);
    g.drawText(labelText, bounds.removeFromLeft(70.0f), juce::Justification::centredLeft);

    // Slider area is handled in resized() - no need to draw here

    // Round counter (e.g., "1/3") or question count - Casino style: Cinzel Art Deco font
    int displayTotal = isQAModeActive
        ? static_cast<int>(qaQuestionsSlider->getValue())
        : static_cast<int>(roundsSlider->getValue());
    g.setColour(tm.getColour(ColourId::TextPrimary));
    g.setFont(fonts.getCasinoTitle(24.0f));  // Cinzel Bold - Art Deco luxury feel

    if (isQAModeActive)
    {
        // Q&A mode: just show the question count
        g.drawText(juce::String(displayTotal),
                   counterBounds, juce::Justification::centredRight);
    }
    else
    {
        // Stars/Guess mode: show current/total rounds
        g.drawText(juce::String(currentRound) + "/" + juce::String(displayTotal),
                   counterBounds, juce::Justification::centredRight);
    }
}

void ControlPanel::drawAutoGainRow(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();
    auto& fonts = FontManager::getInstance();

    // Background bar
    g.setColour(tm.getColour(ColourId::SurfaceAlt));
    g.fillRoundedRectangle(bounds, 8.0f);

    auto contentBounds = bounds.reduced(16.0f, 8.0f);

    // Speaker icon
    g.setColour(tm.getColour(ColourId::TextSecondary));
    g.setFont(fonts.getRegular(18.0f));
    auto iconBounds = contentBounds.removeFromLeft(24.0f);
    g.drawText(juce::String::fromUTF8("🔊"), iconBounds, juce::Justification::centred);

    contentBounds.removeFromLeft(8.0f);  // Gap

    // "LEVEL-MATCH" label - Casino style for English, CJK-compatible for Asian languages
    g.setColour(tm.getColour(ColourId::TextMuted));
    if (isCJKLanguage())
        g.setFont(fonts.getMedium(16.0f));
    else
        g.setFont(fonts.getBebasNeue(14.0f));
    auto labelBounds = contentBounds.removeFromLeft(110.0f);
    g.drawText(LOCALIZE(LabelAutoGain), labelBounds, juce::Justification::centredLeft);

    // Status indicator (between label and toggle)
    // Reserve space for toggle on the right (60px)
    auto statusArea = contentBounds.withTrimmedRight(60.0f);

    if (autoGainEnabled)
    {
        if (calibratingStatus)
        {
            // Calibrating: show pulsing dot and time remaining
            float dotSize = 8.0f;
            auto dotBounds = statusArea.removeFromLeft(dotSize + 8.0f);

            // Pulsing effect using time
            float pulse = 0.5f + 0.5f * std::sin(static_cast<float>(juce::Time::currentTimeMillis()) * 0.005f);
            g.setColour(tm.getColour(ColourId::Primary).withAlpha(0.5f + pulse * 0.5f));
            g.fillEllipse(dotBounds.getCentreX() - dotSize / 2.0f,
                          dotBounds.getCentreY() - dotSize / 2.0f,
                          dotSize, dotSize);

            // "Detecting..." text with time
            g.setColour(tm.getColour(ColourId::Primary));
            if (isCJKLanguage())
                g.setFont(fonts.getMedium(16.0f));
            else
                g.setFont(fonts.getBebasNeue(12.0f));
            juce::String statusText = LOCALIZE(StatusDetecting);
            if (calibrationTimeRemaining > 0.0f)
                statusText = juce::String::formatted("%.1fs", calibrationTimeRemaining);
            g.drawText(statusText, statusArea, juce::Justification::centredLeft);
        }
        else if (calibratedStatus)
        {
            // Calibrated: show checkmark
            g.setColour(tm.getColour(ColourId::Success));
            if (isCJKLanguage())
                g.setFont(fonts.getMedium(16.0f));
            else
                g.setFont(fonts.getBebasNeue(12.0f));
            g.drawText(juce::String::fromUTF8("\xe2\x9c\x93 ") + LOCALIZE(StatusReady), statusArea, juce::Justification::centredLeft);
        }
        else
        {
            // Enabled but waiting for audio
            g.setColour(tm.getColour(ColourId::TextMuted));
            if (isCJKLanguage())
                g.setFont(fonts.getMedium(16.0f));
            else
                g.setFont(fonts.getBebasNeue(12.0f));
            g.drawText(LOCALIZE(StatusWaiting), statusArea, juce::Justification::centredLeft);
        }
    }

    // Toggle is handled in resized()
}

void ControlPanel::resized()
{
    auto bounds = getLocalBounds();
    auto contentBounds = bounds.reduced(20, 16);

    // Top info row
    auto infoRow = contentBounds.removeFromTop(32);

    // Position slider within info row (rounds or Q&A questions depending on mode)
    auto sliderArea = infoRow;
    sliderArea.removeFromLeft(static_cast<int>(infoRow.getWidth() * 0.35f) + 24 + 75 + 8);  // Skip left section + label
    sliderArea.removeFromRight(50);  // Leave space for counter
    sliderArea = sliderArea.withSizeKeepingCentre(sliderWidth, 20);
    roundsSlider->setBounds(sliderArea);
    if (qaQuestionsSlider)
        qaQuestionsSlider->setBounds(sliderArea);

    contentBounds.removeFromTop(16);  // Gap

    // Auto gain row (Level-Match works in both DAW and Standalone modes)
    auto autoGainRow = contentBounds.removeFromTop(40);
    auto toggleArea = autoGainRow.removeFromRight(60).reduced(8, 8);
    autoGainToggle->setBounds(toggleArea);
    contentBounds.removeFromTop(20);  // Gap

    // Button area
    auto buttonArea = contentBounds;

    int buttonSize = ChipButton::kTotalWidth;
    int buttonGap = 24;
    int buttonY = buttonArea.getCentreY() - ChipButton::kTotalHeight / 2;

    // RESET always on the left
    int resetX = buttonArea.getX();
    resetButton->setBounds(resetX, buttonY, buttonSize, ChipButton::kTotalHeight);

    // Count other visible buttons (SHUFFLE, REVEAL, NEXT)
    int otherButtons = 0;
    if (shuffleButton->isVisible()) otherButtons++;
    if (revealButton->isVisible()) otherButtons++;
    if (nextRoundButton->isVisible()) otherButtons++;

    if (otherButtons == 0) otherButtons = 1;  // At least REVEAL

    // Center the other buttons in remaining space
    int otherButtonsWidth = otherButtons * buttonSize + (otherButtons - 1) * buttonGap;
    int remainingAreaStart = resetX + buttonSize + buttonGap;
    int remainingAreaWidth = buttonArea.getRight() - remainingAreaStart;
    int startX = remainingAreaStart + (remainingAreaWidth - otherButtonsWidth) / 2;

    int currentX = startX;

    if (shuffleButton->isVisible())
    {
        shuffleButton->setBounds(currentX, buttonY, buttonSize, ChipButton::kTotalHeight);
        currentX += buttonSize + buttonGap;
    }

    revealButton->setBounds(currentX, buttonY, buttonSize, ChipButton::kTotalHeight);
    currentX += buttonSize + buttonGap;

    if (nextRoundButton->isVisible())
    {
        nextRoundButton->setBounds(currentX, buttonY, buttonSize, ChipButton::kTotalHeight);
    }
}

} // namespace BlindCard
