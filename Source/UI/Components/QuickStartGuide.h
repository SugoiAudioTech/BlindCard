/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    QuickStartGuide.h
    Created: 2026-01-21
    Author:  BlindCard

    Modal dialog showing quick start instructions for new users.
    Triggered by clicking the "?" info button in HeaderBar.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Theme/ThemeManager.h"
#include "../Localization/LocalizationManager.h"

namespace BlindCard
{

/**
 * QuickStartGuide is a modal overlay that shows step-by-step instructions.
 *
 * Design: Scrollable dark background with numbered steps (blue circles),
 * a tip section with light background, and a blue CTA button fixed at bottom.
 */
class QuickStartGuide : public juce::Component,
                        public LocalizationManager::Listener
{
public:
    QuickStartGuide();
    ~QuickStartGuide() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

    // LocalizationManager::Listener
    void languageChanged() override;

    /** Callback when the dialog should be closed */
    std::function<void()> onClose;

    /** Show the guide as an overlay on a parent component */
    void showOverlay(juce::Component* parent);

    /** Hide the overlay */
    void hideOverlay();

private:
    // Layout constants
    static constexpr int kDialogWidth = 680;
    static constexpr int kDialogHeight = 720;
    static constexpr int kCornerRadius = 20;
    static constexpr int kPadding = 36;
    static constexpr int kStepNumberSize = 32;
    static constexpr int kStepSpacing = 36;
    static constexpr int kButtonHeight = 56;
    static constexpr int kButtonWidth = 240;
    static constexpr int kHeaderHeight = 80;      // Title area
    static constexpr int kFooterHeight = 90;      // Button area

    // Colors
    juce::Colour bgColor{ 0xFF1E2530 };           // Dark blue-gray
    juce::Colour borderColor{ 0xFF3A4556 };       // Lighter border
    juce::Colour stepNumberBg{ 0xFF3B82F6 };      // Blue circles
    juce::Colour stepNumberText{ 0xFFFFFFFF };    // White number
    juce::Colour titleColor{ 0xFFFFFFFF };        // White title
    juce::Colour textColor{ 0xFFD1D5DB };         // Light gray text
    juce::Colour tipBgColor{ 0xFF2A3441 };        // Tip box background
    juce::Colour tipBorderColor{ 0xFF3B82F6 };    // Blue tip border
    juce::Colour buttonBgColor{ 0xFF3B82F6 };     // Blue button
    juce::Colour buttonTextColor{ 0xFFFFFFFF };   // White button text

    // UI elements
    juce::Rectangle<int> dialogBounds;
    juce::Rectangle<int> closeButtonBounds;
    juce::Rectangle<int> gotItButtonBounds;
    juce::Rectangle<int> titleBounds;
    juce::Rectangle<int> scrollAreaBounds;

    // Scrollable content
    class ContentComponent : public juce::Component
    {
    public:
        ContentComponent(QuickStartGuide& owner) : parent(owner) {}
        void paint(juce::Graphics& g) override;
        int getContentHeight() const { return contentHeight; }
        void setContentHeight(int h) { contentHeight = h; }
    private:
        QuickStartGuide& parent;
        int contentHeight = 800;
    };

    std::unique_ptr<ContentComponent> contentComponent;
    std::unique_ptr<juce::Viewport> viewport;

    // Drawing helpers (used by ContentComponent)
    void drawDescription(juce::Graphics& g, juce::Rectangle<int>& bounds);
    void drawStep(juce::Graphics& g, juce::Rectangle<int>& bounds, int stepNumber, const juce::String& text);
    void drawStepWithSubitems(juce::Graphics& g, juce::Rectangle<int>& bounds, int stepNumber,
                               const juce::String& intro,
                               const std::vector<std::pair<juce::String, juce::String>>& items);
    void drawTipBox(juce::Graphics& g, juce::Rectangle<int>& bounds);
    void drawKeyboardShortcuts(juce::Graphics& g, juce::Rectangle<int>& bounds);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuickStartGuide)
};

} // namespace BlindCard
