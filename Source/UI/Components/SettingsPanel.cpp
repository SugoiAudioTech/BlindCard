/*
  ==============================================================================

    SettingsPanel.cpp
    Created: 2026-01-21
    Author:  BlindCard

    Implementation of the Settings panel modal dialog.

  ==============================================================================
*/

#include "SettingsPanel.h"

namespace BlindCard
{

SettingsPanel::SettingsPanel()
{
    setOpaque(false);
    setAlwaysOnTop(true);
    setInterceptsMouseClicks(true, true);
}

void SettingsPanel::showOverlay(juce::Component* parent)
{
    if (parent == nullptr) return;

    setBounds(parent->getLocalBounds());
    parent->addAndMakeVisible(this);
    toFront(true);
}

void SettingsPanel::hideOverlay()
{
    setVisible(false);
    if (getParentComponent() != nullptr)
        getParentComponent()->removeChildComponent(this);
}

juce::Font SettingsPanel::getSystemFont(float height, bool bold) const
{
    auto options = juce::FontOptions().withHeight(height);
    if (bold)
        options = options.withStyle("Bold");
    return juce::Font(options);
}

void SettingsPanel::paint(juce::Graphics& g)
{
    // Semi-transparent overlay background
    g.fillAll(juce::Colours::black.withAlpha(0.6f));

    // Dialog box
    g.setColour(bgColor);
    g.fillRoundedRectangle(dialogBounds.toFloat(), static_cast<float>(kCornerRadius));

    // Dialog border
    g.setColour(borderColor);
    g.drawRoundedRectangle(dialogBounds.toFloat().reduced(0.5f),
                            static_cast<float>(kCornerRadius), 1.0f);

    // Title
    g.setColour(titleColor);
    g.setFont(getSystemFont(24.0f, true));
    auto titleBounds = dialogBounds.withHeight(kHeaderHeight).reduced(kPadding, 0);
    titleBounds.removeFromRight(40); // Space for X button
    g.drawText("About", titleBounds, juce::Justification::centredLeft);

    // Close button (X)
    g.setColour(textColor.withAlpha(0.6f));
    g.setFont(getSystemFont(20.0f, true));
    g.drawText("X", closeButtonBounds, juce::Justification::centred);

    // Separator line below title
    g.setColour(borderColor);
    g.fillRect(dialogBounds.getX() + kPadding,
               dialogBounds.getY() + kHeaderHeight - 1,
               dialogBounds.getWidth() - kPadding * 2, 1);

    // Version badge box
    g.setColour(badgeBgColor);
    g.fillRoundedRectangle(versionBadgeBounds.toFloat(), 12.0f);
    g.setColour(accentColor);
    g.drawRoundedRectangle(versionBadgeBounds.toFloat(), 12.0f, 1.5f);

    // Badge content
    auto badgeContent = versionBadgeBounds.reduced(20, 16);

    // Free Software label
    g.setColour(accentColor);
    g.setFont(getSystemFont(18.0f, true));
    g.drawText("It's Free Software", badgeContent.removeFromTop(28), juce::Justification::centredLeft);

    // Subtitle
    g.setColour(juce::Colour(0xFF4ADE80)); // Green color
    g.setFont(getSystemFont(15.0f));
    g.drawText("No license key required", badgeContent, juce::Justification::centredLeft);

    // Bottom info section
    auto bottomSection = dialogBounds.reduced(kPadding);
    bottomSection.removeFromTop(kHeaderHeight + versionBadgeBounds.getHeight() + 30);

    // Separator line
    g.setColour(borderColor);
    g.fillRect(bottomSection.removeFromTop(1));
    bottomSection.removeFromTop(20);

    // Version row
    g.setColour(textColor.withAlpha(0.7f));
    g.setFont(getSystemFont(15.0f));
    auto versionRow = bottomSection.removeFromTop(24);
    g.drawText("Version:", versionRow.removeFromLeft(100), juce::Justification::centredLeft);
    g.setColour(textColor);
    g.drawText(versionString, versionRow, juce::Justification::centredRight);

    // Developer row
    g.setColour(textColor.withAlpha(0.7f));
    auto devRow = bottomSection.removeFromTop(24);
    g.drawText("Developer:", devRow.removeFromLeft(100), juce::Justification::centredLeft);
    g.setColour(textColor);
    g.drawText(developerName, devRow, juce::Justification::centredRight);

    bottomSection.removeFromTop(16);

    // Website link
    g.setColour(linkColor);
    g.setFont(getSystemFont(15.0f));
    g.drawText("Visit Official Website", websiteLinkBounds, juce::Justification::centredLeft);

    // External link indicator arrow
    auto arrowBounds = websiteLinkBounds.withLeft(websiteLinkBounds.getX() + 155).withWidth(20);
    g.drawText("->", arrowBounds, juce::Justification::centredLeft);
}

void SettingsPanel::resized()
{
    auto bounds = getLocalBounds();

    // Center the dialog
    dialogBounds = bounds.withSizeKeepingCentre(kDialogWidth, kDialogHeight);

    // Close button in top-right of dialog
    closeButtonBounds = juce::Rectangle<int>(
        dialogBounds.getRight() - kPadding - 28,
        dialogBounds.getY() + (kHeaderHeight - 28) / 2,
        28, 28
    );

    // Version badge
    versionBadgeBounds = juce::Rectangle<int>(
        dialogBounds.getX() + kPadding,
        dialogBounds.getY() + kHeaderHeight + 20,
        dialogBounds.getWidth() - kPadding * 2,
        80
    );

    // Website link bounds (for click detection)
    websiteLinkBounds = juce::Rectangle<int>(
        dialogBounds.getX() + kPadding,
        dialogBounds.getBottom() - kPadding - 30,
        180,
        24
    );
}

void SettingsPanel::mouseDown(const juce::MouseEvent& event)
{
    auto pos = event.getPosition();

    // Close if clicking X button
    if (closeButtonBounds.contains(pos))
    {
        hideOverlay();
        if (onClose) onClose();
        return;
    }

    // Open website if clicking the link
    if (websiteLinkBounds.contains(pos))
    {
        juce::URL(websiteUrl).launchInDefaultBrowser();
        return;
    }

    // Close if clicking outside dialog
    if (!dialogBounds.contains(pos))
    {
        hideOverlay();
        if (onClose) onClose();
        return;
    }
}

} // namespace BlindCard
