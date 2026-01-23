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

namespace BlindCard
{

/**
 * SettingsPanel is a modal overlay showing version info and links.
 * Similar to AirCheck's Settings & License panel but simplified (no license key).
 */
class SettingsPanel : public juce::Component
{
public:
    SettingsPanel();
    ~SettingsPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

    /** Callback when the dialog should be closed */
    std::function<void()> onClose;

    /** Show the panel as an overlay on a parent component */
    void showOverlay(juce::Component* parent);

    /** Hide the overlay */
    void hideOverlay();

private:
    // Layout constants
    static constexpr int kDialogWidth = 420;
    static constexpr int kDialogHeight = 340;
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
    juce::String versionString = "1.0.0";
    juce::String developerName = "Sugoi Audio";
    juce::String websiteUrl = "https://sugoiaudio.com";

    // Helper to get system font
    juce::Font getSystemFont(float height, bool bold = false) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsPanel)
};

} // namespace BlindCard
