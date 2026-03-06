/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    SettingsPanel.cpp
    Created: 2026-01-21
    Author:  BlindCard

    Implementation of the Settings panel modal dialog.

  ==============================================================================
*/

#include "SettingsPanel.h"
#include <juce_audio_plugin_client/juce_audio_plugin_client.h>

namespace BlindCard
{

SettingsPanel::SettingsPanel()
{
    setOpaque(false);
    setAlwaysOnTop(true);
    setInterceptsMouseClicks(true, true);

    versionString = JucePlugin_VersionString;

    setupLanguageSelector();
    LocalizationManager::getInstance().addListener(this);
}

SettingsPanel::~SettingsPanel()
{
    LocalizationManager::getInstance().removeListener(this);
}

void SettingsPanel::setUpdateInfo(bool available, const juce::String& latestVer, const juce::String& url,
                                   const juce::String& changelog)
{
    updateAvailable = available;
    latestVersionString = latestVer;
    downloadUrl = url;
    changelogText = changelog;
    repaint();
}

void SettingsPanel::setupLanguageSelector()
{
    // Use TextButtons instead of ComboBox to avoid native popup window issues
    // in AU/Logic Pro. ComboBox popups create NSWindow that can corrupt
    // AUHostingServiceXPC window management and prevent UI from reopening.
    struct LangInfo { const char* name; };
    LangInfo langs[] = {
        { "EN" },
        { "\xe7\xb9\x81" },   // 繁
        { "\xe7\xae\x80" },   // 简
        { "\xe6\x97\xa5" },   // 日
        { "\xed\x95\x9c" },   // 한
    };

    for (int i = 0; i < 5; ++i)
    {
        auto* btn = languageButtons.add(new juce::TextButton(juce::CharPointer_UTF8(langs[i].name)));
        btn->setClickingTogglesState(false);
        btn->onClick = [this, i]() { onLanguageSelected(i); };
        addAndMakeVisible(btn);
    }

    updateLanguageButtonStates();
}

void SettingsPanel::onLanguageSelected(int langIndex)
{
    Language langs[] = {
        Language::English,
        Language::TraditionalChinese,
        Language::SimplifiedChinese,
        Language::Japanese,
        Language::Korean
    };

    if (langIndex >= 0 && langIndex < 5)
        LocalizationManager::getInstance().setLanguage(langs[langIndex]);

    updateLanguageButtonStates();
}

void SettingsPanel::updateLanguageButtonStates()
{
    auto currentLang = LocalizationManager::getInstance().getCurrentLanguage();
    int selectedIndex = static_cast<int>(currentLang);  // enum maps to 0-4

    for (int i = 0; i < languageButtons.size(); ++i)
    {
        bool isSelected = (i == selectedIndex);
        languageButtons[i]->setToggleState(isSelected, juce::dontSendNotification);

        // Visual feedback: selected button gets accent color
        if (isSelected)
        {
            languageButtons[i]->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF3B82F6));
            languageButtons[i]->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        }
        else
        {
            languageButtons[i]->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2A3040));
            languageButtons[i]->setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFD1D5DB));
        }
    }
}

void SettingsPanel::languageChanged()
{
    updateLanguageButtonStates();
    repaint();
    if (onLanguageChanged) onLanguageChanged();
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
    g.drawText(LOCALIZE(AboutTitle), titleBounds, juce::Justification::centredLeft);

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
    g.drawText(LOCALIZE(AboutFreeSoftware), badgeContent.removeFromTop(28), juce::Justification::centredLeft);

    // Subtitle
    g.setColour(juce::Colour(0xFF4ADE80)); // Green color
    g.setFont(getSystemFont(15.0f));
    g.drawText(LOCALIZE(AboutNoLicense), badgeContent, juce::Justification::centredLeft);

    // Bottom info section
    auto bottomSection = dialogBounds.reduced(kPadding);
    bottomSection.removeFromTop(kHeaderHeight + versionBadgeBounds.getHeight() + 30);

    // Separator line
    g.setColour(borderColor);
    g.fillRect(bottomSection.removeFromTop(1));
    bottomSection.removeFromTop(20);

    // Language selector label
    g.setColour(textColor.withAlpha(0.7f));
    g.setFont(getSystemFont(15.0f));
    g.drawText(LOCALIZE(SettingsLanguage), languageLabelBounds, juce::Justification::centredLeft);

    bottomSection.removeFromTop(48); // Space for language combo

    // Separator line before info
    g.setColour(borderColor);
    g.fillRect(bottomSection.removeFromTop(1));
    bottomSection.removeFromTop(16);

    // Version row
    g.setColour(textColor.withAlpha(0.7f));
    g.setFont(getSystemFont(15.0f));
    auto versionRow = bottomSection.removeFromTop(24);
    g.drawText(LOCALIZE(AboutVersion), versionRow.removeFromLeft(100), juce::Justification::centredLeft);
    g.setColour(textColor);
    g.drawText(versionString, versionRow, juce::Justification::centredRight);

    // Update status row
    auto updateRow = bottomSection.removeFromTop(24);
    if (updateAvailable && latestVersionString.isNotEmpty())
    {
        g.setColour(juce::Colour(0xFF22C55E));  // Green
        g.setFont(getSystemFont(14.0f, true));
        juce::String updateText = LOCALIZE(UpdateAvailable) + " v" + latestVersionString;
        g.drawText(updateText, updateRow.removeFromLeft(updateRow.getWidth() - 80), juce::Justification::centredLeft);

        updateLinkBounds = updateRow;
        g.setColour(linkColor);
        g.setFont(getSystemFont(14.0f));
        g.drawText(LOCALIZE(UpdateDownload), updateLinkBounds, juce::Justification::centredRight);
    }
    else
    {
        g.setColour(textColor.withAlpha(0.5f));
        g.setFont(getSystemFont(13.0f));
        g.drawText(LOCALIZE(UpdateUpToDate), updateRow, juce::Justification::centredLeft);
        updateLinkBounds = {};
    }

    // Changelog row (if update available and changelog exists)
    if (updateAvailable && changelogText.isNotEmpty())
    {
        auto changelogRow = bottomSection.removeFromTop(20);
        g.setColour(textColor.withAlpha(0.5f));
        g.setFont(getSystemFont(12.0f));
        g.drawText(changelogText, changelogRow, juce::Justification::centredLeft);
    }

    // Developer row
    g.setColour(textColor.withAlpha(0.7f));
    auto devRow = bottomSection.removeFromTop(24);
    g.drawText(LOCALIZE(AboutDeveloper), devRow.removeFromLeft(100), juce::Justification::centredLeft);
    g.setColour(textColor);
    g.drawText(developerName, devRow, juce::Justification::centredRight);

    bottomSection.removeFromTop(16);

    // Website link
    g.setColour(linkColor);
    g.setFont(getSystemFont(15.0f));
    g.drawText(websiteUrl, websiteLinkBounds, juce::Justification::centredLeft);
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

    // Language selector section (after badge + separator)
    int languageSectionY = versionBadgeBounds.getBottom() + 50;
    languageLabelBounds = juce::Rectangle<int>(
        dialogBounds.getX() + kPadding,
        languageSectionY,
        100,
        24
    );

    // Language buttons (row of 5 buttons)
    {
        int btnX = dialogBounds.getX() + kPadding + 110;
        int btnY = languageSectionY - 2;
        int availWidth = dialogBounds.getWidth() - kPadding * 2 - 110;
        int btnWidth = (availWidth - 4 * 4) / 5;  // 4px gap between buttons
        int btnHeight = 28;

        for (int i = 0; i < languageButtons.size(); ++i)
        {
            languageButtons[i]->setBounds(btnX + i * (btnWidth + 4), btnY, btnWidth, btnHeight);
        }
    }

    // Website link bounds (for click detection)
    websiteLinkBounds = juce::Rectangle<int>(
        dialogBounds.getX() + kPadding,
        dialogBounds.getBottom() - kPadding - 30,
        220,
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

    // Open update download link
    if (updateLinkBounds.getWidth() > 0 && updateLinkBounds.contains(pos))
    {
        if (downloadUrl.isNotEmpty())
            juce::URL(downloadUrl).launchInDefaultBrowser();
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
