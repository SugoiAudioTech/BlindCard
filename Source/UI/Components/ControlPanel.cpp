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

    // Thumb body
    g.setColour(juce::Colours::white);
    g.fillEllipse(thumbBounds);
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

    // Create action buttons (only REVEAL and RESET visible initially)
    shuffleButton = std::make_unique<ChipButton>("SHUFFLE", ChipVariant::Red);
    shuffleButton->onClick = [this]()
    {
        if (onShuffleClicked)
            onShuffleClicked();
    };
    addAndMakeVisible(*shuffleButton);
    shuffleButton->setVisible(false);  // Hidden by default

    revealButton = std::make_unique<ChipButton>("REVEAL", ChipVariant::Blue);
    revealButton->onClick = [this]()
    {
        if (onRevealClicked)
            onRevealClicked();
    };
    addAndMakeVisible(*revealButton);

    resetButton = std::make_unique<ChipButton>("RESET", ChipVariant::Black);
    resetButton->onClick = [this]()
    {
        if (onResetClicked)
            onResetClicked();
    };
    addAndMakeVisible(*resetButton);

    nextRoundButton = std::make_unique<ChipButton>("NEXT", ChipVariant::Gold);
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
    roundsSlider->setLookAndFeel(nullptr);
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

void ControlPanel::setAutoGain(bool enabled)
{
    autoGainEnabled = enabled;
    autoGainToggle->setToggleState(enabled, juce::dontSendNotification);
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

    // Auto gain row
    auto autoGainRow = contentBounds.removeFromTop(40.0f);
    drawAutoGainRow(g, autoGainRow);
}

void ControlPanel::drawInfoRow(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();

    // Use system sans-serif font for clean modern look
    auto sansSerif = juce::Font::getDefaultSansSerifFontName();

    // Left section: Current Tracks
    auto leftSection = bounds.removeFromLeft(bounds.getWidth() * 0.35f);

    // Music note icon
    g.setColour(tm.getColour(ColourId::Primary));
    g.setFont(juce::Font(sansSerif, 16.0f, juce::Font::plain));
    auto iconBounds = leftSection.removeFromLeft(24.0f);
    g.drawText(juce::String::fromUTF8("♪"), iconBounds, juce::Justification::centred);

    leftSection.removeFromLeft(8.0f);  // Gap

    // "Current Tracks" label
    g.setColour(tm.getColour(ColourId::TextSecondary));
    g.setFont(juce::Font(sansSerif, 13.0f, juce::Font::plain));
    auto labelBounds = leftSection.removeFromLeft(100.0f);
    g.drawText("Current Tracks", labelBounds, juce::Justification::centredLeft);

    // Track count in gold circle
    int trackCount = static_cast<int>(trackNames.size());
    if (trackCount == 0) trackCount = 4;  // Default display

    auto countBounds = leftSection.removeFromLeft(28.0f).reduced(2.0f);
    g.setColour(tm.getColour(ColourId::Accent));
    g.fillEllipse(countBounds);
    g.setColour(juce::Colours::black);
    g.setFont(juce::Font(sansSerif, 12.0f, juce::Font::bold));
    g.drawText(juce::String(trackCount), countBounds, juce::Justification::centred);

    bounds.removeFromLeft(24.0f);  // Gap

    // Right section: Rounds
    // Hash icon
    g.setColour(tm.getColour(ColourId::TextSecondary));
    g.setFont(juce::Font(sansSerif, 14.0f, juce::Font::bold));
    auto hashBounds = bounds.removeFromLeft(20.0f);
    g.drawText("#", hashBounds, juce::Justification::centred);

    bounds.removeFromLeft(4.0f);  // Gap

    // "Rounds" label
    g.setFont(juce::Font(sansSerif, 13.0f, juce::Font::plain));
    auto roundsLabelBounds = bounds.removeFromLeft(50.0f);
    g.drawText("Rounds", roundsLabelBounds, juce::Justification::centredLeft);

    // Slider area is handled in resized()
    bounds.removeFromLeft(sliderWidth + 16.0f);

    // Round counter (e.g., "1/3")
    int currentRound = 1;  // This would come from manager
    int totalRounds = static_cast<int>(roundsSlider->getValue());
    g.setColour(tm.getColour(ColourId::TextPrimary));
    g.setFont(juce::Font(sansSerif, 13.0f, juce::Font::plain));
    g.drawText(juce::String(currentRound) + "/" + juce::String(totalRounds),
               bounds, juce::Justification::centredRight);
}

void ControlPanel::drawAutoGainRow(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();

    // Use system sans-serif font
    auto sansSerif = juce::Font::getDefaultSansSerifFontName();

    // Background bar
    g.setColour(tm.getColour(ColourId::SurfaceAlt));
    g.fillRoundedRectangle(bounds, 8.0f);

    auto contentBounds = bounds.reduced(16.0f, 8.0f);

    // Speaker icon
    g.setColour(tm.getColour(ColourId::TextSecondary));
    g.setFont(juce::Font(sansSerif, 16.0f, juce::Font::plain));
    auto iconBounds = contentBounds.removeFromLeft(24.0f);
    g.drawText(juce::String::fromUTF8("🔊"), iconBounds, juce::Justification::centred);

    contentBounds.removeFromLeft(8.0f);  // Gap

    // "Auto Gain" label
    g.setColour(tm.getColour(ColourId::TextPrimary));
    g.setFont(juce::Font(sansSerif, 14.0f, juce::Font::plain));
    g.drawText("Auto Gain", contentBounds, juce::Justification::centredLeft);

    // Toggle is handled in resized()
}

void ControlPanel::resized()
{
    auto bounds = getLocalBounds();
    auto contentBounds = bounds.reduced(20, 16);

    // Top info row
    auto infoRow = contentBounds.removeFromTop(32);

    // Position rounds slider within info row
    auto sliderArea = infoRow;
    sliderArea.removeFromLeft(static_cast<int>(infoRow.getWidth() * 0.35f) + 24 + 20 + 4 + 50 + 8);  // Skip left section
    sliderArea.removeFromRight(50);  // Leave space for counter
    sliderArea = sliderArea.withSizeKeepingCentre(sliderWidth, 20);
    roundsSlider->setBounds(sliderArea);

    contentBounds.removeFromTop(16);  // Gap

    // Auto gain row
    auto autoGainRow = contentBounds.removeFromTop(40);
    auto toggleArea = autoGainRow.removeFromRight(60).reduced(8, 8);
    autoGainToggle->setBounds(toggleArea);

    contentBounds.removeFromTop(20);  // Gap

    // Button area
    auto buttonArea = contentBounds;

    // Count visible buttons
    int visibleButtons = 0;
    if (shuffleButton->isVisible()) visibleButtons++;
    if (revealButton->isVisible()) visibleButtons++;
    if (resetButton->isVisible()) visibleButtons++;
    if (nextRoundButton->isVisible()) visibleButtons++;

    if (visibleButtons == 0) visibleButtons = 2;  // Default to REVEAL and RESET

    int buttonSize = ChipButton::kTotalWidth;
    int buttonGap = 24;
    int totalButtonsWidth = visibleButtons * buttonSize + (visibleButtons - 1) * buttonGap;
    int startX = buttonArea.getCentreX() - totalButtonsWidth / 2;
    int buttonY = buttonArea.getCentreY() - ChipButton::kTotalHeight / 2;

    int currentX = startX;

    if (shuffleButton->isVisible())
    {
        shuffleButton->setBounds(currentX, buttonY, buttonSize, ChipButton::kTotalHeight);
        currentX += buttonSize + buttonGap;
    }

    revealButton->setBounds(currentX, buttonY, buttonSize, ChipButton::kTotalHeight);
    currentX += buttonSize + buttonGap;

    resetButton->setBounds(currentX, buttonY, buttonSize, ChipButton::kTotalHeight);
    currentX += buttonSize + buttonGap;

    if (nextRoundButton->isVisible())
    {
        nextRoundButton->setBounds(currentX, buttonY, buttonSize, ChipButton::kTotalHeight);
    }
}

} // namespace BlindCard
