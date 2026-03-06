/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    SettingsPanel.h
    Created: 2026-01-21
    Author:  BlindCard

    Modal dialog showing plugin info and settings.
    Triggered by clicking the gear icon in HeaderBar.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Localization/LocalizationManager.h"

namespace BlindCard
{

/**
 * SettingsPanel is a modal overlay showing version info, language settings, and links.
 * Similar to AirCheck's Settings & License panel but simplified (no license key).
 */
class SettingsPanel : public juce::Component,
                      public LocalizationManager::Listener
{
public:
    SettingsPanel();
    ~SettingsPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

    // LocalizationManager::Listener
    void languageChanged() override;

    /** Callback when the dialog should be closed */
    std::function<void()> onClose;

    /** Callback when language changes (for parent to refresh UI) */
    std::function<void()> onLanguageChanged;

    /** Set the update check state for display */
    void setUpdateInfo(bool available, const juce::String& latestVer, const juce::String& url,
                       const juce::String& changelog = {});

    /** Show the panel as an overlay on a parent component */
    void showOverlay(juce::Component* parent);

    /** Hide the overlay */
    void hideOverlay();

private:
    // Layout constants
    static constexpr int kDialogWidth = 420;
    static constexpr int kDialogHeight = 450;  // Increased for language selector + update row
    static constexpr int kCornerRadius = 16;
    static constexpr int kPadding = 28;
    static constexpr int kHeaderHeight = 60;

    // Colors (matching QuickStartGuide)
    juce::Colour bgColor{ 0xFF1E2530 };
    juce::Colour borderColor{ 0xFF3A4556 };
    juce::Colour titleColor{ 0xFFFFFFFF };
    juce::Colour textColor{ 0xFFD1D5DB };
    juce::Colour accentColor{ 0xFF3B82F6 };      // Blue
    juce::Colour badgeBgColor{ 0xFF1E3A5F };     // Dark blue for badge
    juce::Colour linkColor{ 0xFF60A5FA };        // Light blue for links

    // UI bounds
    juce::Rectangle<int> dialogBounds;
    juce::Rectangle<int> closeButtonBounds;
    juce::Rectangle<int> versionBadgeBounds;
    juce::Rectangle<int> websiteLinkBounds;

    // Version info
    juce::String versionString;
    juce::String developerName = "Sugoi Audio";
    juce::String websiteUrl = "https://sugoiaudio.com";

    // Update check state
    bool updateAvailable = false;
    juce::String latestVersionString;
    juce::String downloadUrl;
    juce::String changelogText;
    juce::Rectangle<int> updateLinkBounds;

    // Helper to get system font
    juce::Font getSystemFont(float height, bool bold = false) const;

    // Language selector (TextButtons instead of ComboBox to avoid native popup
    // window issues in AU/Logic Pro — ComboBox popups create NSWindow that can
    // corrupt AUHostingServiceXPC window management)
    juce::OwnedArray<juce::TextButton> languageButtons;
    juce::Rectangle<int> languageLabelBounds;
    void setupLanguageSelector();
    void onLanguageSelected(int langIndex);
    void updateLanguageButtonStates();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsPanel)
};

} // namespace BlindCard
