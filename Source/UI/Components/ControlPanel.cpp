/*
  ==============================================================================

    ControlPanel.cpp
    Created: 2026-01-19
    Author:  BlindCard

  ==============================================================================
*/

#include "ControlPanel.h"

namespace BlindCard
{

//==============================================================================
// Custom slider look and feel
void ControlPanel::CustomSliderLookAndFeel::drawLinearSlider(
    juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
    const juce::Slider::SliderStyle, juce::Slider& slider)
{
    auto& tm = ThemeManager::getInstance();

    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    auto trackHeight = 6.0f;
    auto trackBounds = bounds.withSizeKeepingCentre(bounds.getWidth(), trackHeight);

    // Track background
    g.setColour(tm.getColour(ColourId::SliderTrack));
    g.fillRoundedRectangle(trackBounds, trackHeight / 2.0f);

    // Filled portion
    auto filledWidth = sliderPos - static_cast<float>(x);
    auto filledBounds = trackBounds.withWidth(filledWidth);
    g.setColour(tm.getColour(ColourId::SliderFill));
    g.fillRoundedRectangle(filledBounds, trackHeight / 2.0f);

    // Thumb
    auto thumbSize = 18.0f;
    auto thumbBounds = juce::Rectangle<float>(thumbSize, thumbSize)
                           .withCentre({ sliderPos, bounds.getCentreY() });

    // Thumb shadow
    g.setColour(juce::Colours::black.withAlpha(0.2f));
    g.fillEllipse(thumbBounds.translated(0.0f, 2.0f));

    // Thumb body
    g.setColour(juce::Colours::white);
    g.fillEllipse(thumbBounds);

    // Thumb border
    g.setColour(tm.getColour(ColourId::SliderFill));
    g.drawEllipse(thumbBounds, 2.0f);
}

//==============================================================================
ControlPanel::ControlPanel()
{
    ThemeManager::getInstance().addChangeListener(this);

    // Create custom look and feel
    sliderLookAndFeel = std::make_unique<CustomSliderLookAndFeel>();

    // Create rounds slider
    roundsSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal,
                                                   juce::Slider::TextBoxRight);
    roundsSlider->setRange(2.0, 8.0, 1.0);
    roundsSlider->setValue(4.0);
    roundsSlider->setLookAndFeel(sliderLookAndFeel.get());
    roundsSlider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 30, 20);
    roundsSlider->onValueChange = [this]()
    {
        if (onRoundsChanged)
            onRoundsChanged(static_cast<int>(roundsSlider->getValue()));
    };
    addAndMakeVisible(*roundsSlider);

    // Create auto gain toggle
    autoGainToggle = std::make_unique<juce::ToggleButton>("Auto Gain");
    autoGainToggle->setToggleState(true, juce::dontSendNotification);
    autoGainToggle->onClick = [this]()
    {
        autoGainEnabled = autoGainToggle->getToggleState();
        if (onAutoGainChanged)
            onAutoGainChanged(autoGainEnabled);
    };
    addAndMakeVisible(*autoGainToggle);

    // Create action buttons
    shuffleButton = std::make_unique<ChipButton>("SHUFFLE", ChipVariant::Red);
    shuffleButton->setIcon(juce::String::fromUTF8("🔀"));
    shuffleButton->onClick = [this]()
    {
        if (onShuffleClicked)
            onShuffleClicked();
    };
    addAndMakeVisible(*shuffleButton);

    revealButton = std::make_unique<ChipButton>("REVEAL", ChipVariant::Blue);
    revealButton->setIcon(juce::String::fromUTF8("👁"));
    revealButton->onClick = [this]()
    {
        if (onRevealClicked)
            onRevealClicked();
    };
    addAndMakeVisible(*revealButton);

    resetButton = std::make_unique<ChipButton>("RESET", ChipVariant::Black);
    resetButton->setIcon(juce::String::fromUTF8("↺"));
    resetButton->onClick = [this]()
    {
        if (onResetClicked)
            onResetClicked();
    };
    addAndMakeVisible(*resetButton);

    nextRoundButton = std::make_unique<ChipButton>("NEXT", ChipVariant::Gold);
    nextRoundButton->setIcon(juce::String::fromUTF8("→"));
    nextRoundButton->onClick = [this]()
    {
        if (onNextRoundClicked)
            onNextRoundClicked();
    };
    addAndMakeVisible(*nextRoundButton);
}

ControlPanel::~ControlPanel()
{
    ThemeManager::getInstance().removeChangeListener(this);
    roundsSlider->setLookAndFeel(nullptr);
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
}

int ControlPanel::getRounds() const
{
    return static_cast<int>(roundsSlider->getValue());
}

void ControlPanel::setAutoGain(bool enabled)
{
    autoGainEnabled = enabled;
    autoGainToggle->setToggleState(enabled, juce::dontSendNotification);
}

//==============================================================================
void ControlPanel::setShuffleEnabled(bool enabled)
{
    shuffleButton->setEnabled(enabled);
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
}

//==============================================================================
void ControlPanel::changeListenerCallback(juce::ChangeBroadcaster*)
{
    repaint();
}

//==============================================================================
void ControlPanel::paint(juce::Graphics& g)
{
    auto& tm = ThemeManager::getInstance();
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(tm.getColour(ColourId::Background));
    g.fillRect(bounds);

    // Sections
    auto contentBounds = bounds.reduced(16.0f, 12.0f);
    auto buttonArea = contentBounds.removeFromBottom(static_cast<float>(kButtonAreaHeight));
    contentBounds.removeFromBottom(12.0f);  // Gap

    // Split remaining area
    auto tracksArea = contentBounds.removeFromLeft(contentBounds.getWidth() * 0.5f - 8.0f);
    contentBounds.removeFromLeft(16.0f);  // Gap
    auto settingsArea = contentBounds;

    // Draw sections
    drawSectionBackground(g, tracksArea, "TRACKS");
    drawSectionBackground(g, settingsArea, "SETTINGS");

    // Draw tracks list
    auto tracksContentArea = tracksArea.reduced(12.0f).withTrimmedTop(24.0f);
    drawTracksList(g, tracksContentArea);
}

void ControlPanel::drawSectionBackground(juce::Graphics& g, juce::Rectangle<float> bounds,
                                          const juce::String& title)
{
    auto& tm = ThemeManager::getInstance();

    // Section background
    g.setColour(tm.getColour(ColourId::Surface));
    g.fillRoundedRectangle(bounds, 8.0f);

    // Section title
    auto titleBounds = bounds.removeFromTop(28.0f).reduced(12.0f, 4.0f);
    g.setColour(tm.getColour(ColourId::TextSecondary));
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText(title, titleBounds, juce::Justification::centredLeft);
}

void ControlPanel::drawTracksList(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();

    g.setFont(juce::Font(12.0f));
    float lineHeight = 20.0f;

    for (size_t i = 0; i < trackNames.size() && i < 8; ++i)
    {
        auto lineBounds = bounds.removeFromTop(lineHeight);

        // Index number
        g.setColour(tm.getColour(ColourId::TextMuted));
        auto indexArea = lineBounds.removeFromLeft(20.0f);
        g.drawText(juce::String(i + 1) + ".", indexArea, juce::Justification::centredLeft);

        // Track name
        g.setColour(tm.getColour(ColourId::TextPrimary));
        g.drawText(juce::String(trackNames[i]), lineBounds, juce::Justification::centredLeft);
    }

    if (trackNames.empty())
    {
        g.setColour(tm.getColour(ColourId::TextMuted));
        g.setFont(juce::Font(11.0f, juce::Font::italic));
        g.drawText("No tracks loaded", bounds, juce::Justification::centred);
    }
}

void ControlPanel::resized()
{
    auto bounds = getLocalBounds();
    auto contentBounds = bounds.reduced(16, 12);

    // Button area at bottom
    auto buttonArea = contentBounds.removeFromBottom(kButtonAreaHeight);
    contentBounds.removeFromBottom(12);  // Gap

    // Split for tracks and settings
    auto tracksArea = contentBounds.removeFromLeft(contentBounds.getWidth() / 2 - 8);
    contentBounds.removeFromLeft(16);  // Gap
    auto settingsArea = contentBounds;

    // Settings area layout
    auto settingsContentArea = settingsArea.reduced(12).withTrimmedTop(28);

    // Rounds slider
    auto roundsArea = settingsContentArea.removeFromTop(40);
    auto roundsLabelArea = roundsArea.removeFromTop(16);
    roundsSlider->setBounds(roundsArea.reduced(0, 4));

    settingsContentArea.removeFromTop(12);  // Gap

    // Auto gain toggle
    auto toggleArea = settingsContentArea.removeFromTop(32);
    autoGainToggle->setBounds(toggleArea);

    // Button layout (centered)
    int buttonSize = ChipButton::kTotalWidth;
    int buttonGap = 16;
    int totalButtonsWidth = 4 * buttonSize + 3 * buttonGap;
    int startX = buttonArea.getCentreX() - totalButtonsWidth / 2;
    int buttonY = buttonArea.getCentreY() - ChipButton::kTotalHeight / 2;

    shuffleButton->setBounds(startX, buttonY, buttonSize, ChipButton::kTotalHeight);
    revealButton->setBounds(startX + buttonSize + buttonGap, buttonY, buttonSize, ChipButton::kTotalHeight);
    resetButton->setBounds(startX + 2 * (buttonSize + buttonGap), buttonY, buttonSize, ChipButton::kTotalHeight);
    nextRoundButton->setBounds(startX + 3 * (buttonSize + buttonGap), buttonY, buttonSize, ChipButton::kTotalHeight);
}

} // namespace BlindCard
