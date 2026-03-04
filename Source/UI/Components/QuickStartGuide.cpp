/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    QuickStartGuide.cpp
    Created: 2026-01-21
    Author:  BlindCard

    Implementation of the Quick Start Guide modal dialog with scrollable content.

  ==============================================================================
*/

#include "QuickStartGuide.h"

namespace BlindCard
{

namespace
{
    // System font helpers for native look (SF Pro on macOS, Segoe UI on Windows)
    juce::Font getSystemFont(float height, bool bold = false)
    {
        auto options = juce::FontOptions()
            .withHeight(height);
        if (bold)
            options = options.withStyle("Bold");
        return juce::Font(options);
    }
}

QuickStartGuide::QuickStartGuide()
{
    setOpaque(false);
    setAlwaysOnTop(true);
    setInterceptsMouseClicks(true, true); // Allow child components to receive clicks

    // Create scrollable content
    contentComponent = std::make_unique<ContentComponent>(*this);
    contentComponent->setOpaque(false);

    viewport = std::make_unique<juce::Viewport>();
    viewport->setViewedComponent(contentComponent.get(), false);
    viewport->setScrollBarsShown(true, false);
    viewport->setScrollOnDragMode(juce::Viewport::ScrollOnDragMode::all); // Enable drag scrolling
    viewport->getVerticalScrollBar().setColour(juce::ScrollBar::thumbColourId, juce::Colour(0xFF4A5568));
    viewport->getVerticalScrollBar().setAutoHide(false); // Always show scrollbar
    addAndMakeVisible(viewport.get());

    // Register for language changes
    LocalizationManager::getInstance().addListener(this);
}

QuickStartGuide::~QuickStartGuide()
{
    LocalizationManager::getInstance().removeListener(this);
}

void QuickStartGuide::languageChanged()
{
    repaint();
    if (contentComponent)
        contentComponent->repaint();
}

void QuickStartGuide::showOverlay(juce::Component* parent)
{
    if (parent == nullptr) return;

    setBounds(parent->getLocalBounds());
    parent->addAndMakeVisible(this);
    toFront(true);

    // Reset scroll position
    viewport->setViewPosition(0, 0);
}

void QuickStartGuide::hideOverlay()
{
    setVisible(false);
    if (getParentComponent() != nullptr)
        getParentComponent()->removeChildComponent(this);
}

void QuickStartGuide::paint(juce::Graphics& g)
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

    // Title area background (fixed header)
    auto headerArea = dialogBounds.withHeight(kHeaderHeight);
    g.setColour(bgColor);
    g.fillRoundedRectangle(headerArea.toFloat(), static_cast<float>(kCornerRadius));

    // Title text
    g.setColour(titleColor);
    g.setFont(getSystemFont(28.0f, true));
    auto titleTextBounds = headerArea.reduced(kPadding, 0);
    titleTextBounds.removeFromRight(50); // Space for X button
    g.drawText(LOCALIZE(GuideTitle), titleTextBounds, juce::Justification::centredLeft);

    // Separator line below title
    g.setColour(borderColor);
    g.fillRect(dialogBounds.getX() + kPadding, dialogBounds.getY() + kHeaderHeight - 1,
               dialogBounds.getWidth() - kPadding * 2, 1);

    // Footer area background (fixed bottom)
    auto footerArea = dialogBounds.withTop(dialogBounds.getBottom() - kFooterHeight);
    g.setColour(bgColor);
    g.fillRect(footerArea);

    // Separator line above footer
    g.setColour(borderColor);
    g.fillRect(footerArea.getX() + kPadding, footerArea.getY(),
               footerArea.getWidth() - kPadding * 2, 1);

    // Got it button
    g.setColour(buttonBgColor);
    g.fillRoundedRectangle(gotItButtonBounds.toFloat(), 12.0f);
    g.setColour(buttonTextColor);
    g.setFont(getSystemFont(20.0f, true));
    g.drawText(LOCALIZE(GuideGotIt), gotItButtonBounds, juce::Justification::centred);

    // Close button (X)
    g.setColour(textColor.withAlpha(0.6f));
    g.setFont(getSystemFont(24.0f, true));
    g.drawText("X", closeButtonBounds, juce::Justification::centred);
}

void QuickStartGuide::resized()
{
    auto bounds = getLocalBounds();

    // Center the dialog
    dialogBounds = bounds.withSizeKeepingCentre(kDialogWidth, kDialogHeight);

    // Close button in top-right of dialog
    closeButtonBounds = juce::Rectangle<int>(
        dialogBounds.getRight() - kPadding - 32,
        dialogBounds.getY() + (kHeaderHeight - 32) / 2,
        32, 32
    );

    // Got it button at bottom center
    gotItButtonBounds = juce::Rectangle<int>(
        dialogBounds.getCentreX() - kButtonWidth / 2,
        dialogBounds.getBottom() - kFooterHeight + (kFooterHeight - kButtonHeight) / 2,
        kButtonWidth,
        kButtonHeight
    );

    // Scroll area (between header and footer)
    scrollAreaBounds = juce::Rectangle<int>(
        dialogBounds.getX() + kPadding,
        dialogBounds.getY() + kHeaderHeight,
        dialogBounds.getWidth() - kPadding * 2,
        dialogBounds.getHeight() - kHeaderHeight - kFooterHeight
    );

    viewport->setBounds(scrollAreaBounds);

    // Set content component size (taller than viewport for scrolling)
    int contentHeight = 1150; // Enough for all content including Tip box and Keyboard Shortcuts
    contentComponent->setSize(scrollAreaBounds.getWidth() - 16, contentHeight); // -16 for scrollbar
    contentComponent->setContentHeight(contentHeight);
}

void QuickStartGuide::mouseDown(const juce::MouseEvent& event)
{
    auto pos = event.getPosition();

    // Close if clicking X button
    if (closeButtonBounds.contains(pos))
    {
        hideOverlay();
        if (onClose) onClose();
        return;
    }

    // Close if clicking Got it button
    if (gotItButtonBounds.contains(pos))
    {
        hideOverlay();
        if (onClose) onClose();
        return;
    }

    // Close if clicking outside dialog
    if (!dialogBounds.contains(pos))
    {
        hideOverlay();
        if (onClose) onClose();
        return;
    }

    // Don't consume clicks in the scroll area - let viewport handle them
    // (This is handled automatically since viewport is a child component)
}

//==============================================================================
// ContentComponent - Scrollable content

void QuickStartGuide::ContentComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(20); // Top padding

    // Description
    parent.drawDescription(g, bounds);
    bounds.removeFromTop(32);

    // Steps with large fonts
    parent.drawStep(g, bounds, 1, LOCALIZE(GuideStep1Full));
    bounds.removeFromTop(parent.kStepSpacing);

    // Step 2 with sub-items
    std::vector<std::pair<juce::String, juce::String>> modeItems = {
        { LOCALIZE(ModeStars), LOCALIZE(GuideModeStarsDesc) },
        { LOCALIZE(ModeGuess), LOCALIZE(GuideModeGuessDesc) },
        { LOCALIZE(ModeQA), LOCALIZE(GuideModeQADesc) }
    };
    parent.drawStepWithSubitems(g, bounds, 2, LOCALIZE(GuideStep2Intro), modeItems);
    bounds.removeFromTop(parent.kStepSpacing);

    parent.drawStep(g, bounds, 3, LOCALIZE(GuideStep3Full));
    bounds.removeFromTop(parent.kStepSpacing);

    parent.drawStep(g, bounds, 4, LOCALIZE(GuideStep4Full));
    bounds.removeFromTop(parent.kStepSpacing);

    parent.drawStep(g, bounds, 5, LOCALIZE(GuideStep5Full));
    bounds.removeFromTop(parent.kStepSpacing);

    parent.drawStep(g, bounds, 6, LOCALIZE(GuideStep6Full));
    bounds.removeFromTop(44);

    // Tip box
    parent.drawTipBox(g, bounds);
    bounds.removeFromTop(32);

    // Keyboard shortcuts
    parent.drawKeyboardShortcuts(g, bounds);
}

//==============================================================================
// Drawing helpers

void QuickStartGuide::drawDescription(juce::Graphics& g, juce::Rectangle<int>& bounds)
{
    g.setColour(textColor);
    g.setFont(getSystemFont(20.0f));

    auto descBounds = bounds.removeFromTop(60);
    g.drawFittedText(
        LOCALIZE(GuideDescription),
        descBounds,
        juce::Justification::topLeft,
        3
    );
}

void QuickStartGuide::drawStep(juce::Graphics& g, juce::Rectangle<int>& bounds, int stepNumber, const juce::String& text)
{
    auto stepBounds = bounds.removeFromTop(64);

    // Number circle
    auto circleBounds = stepBounds.removeFromLeft(kStepNumberSize).toFloat();
    // Center vertically
    circleBounds = circleBounds.withY(circleBounds.getY() + (64 - kStepNumberSize) / 2.0f)
                               .withHeight(static_cast<float>(kStepNumberSize));
    g.setColour(stepNumberBg);
    g.fillEllipse(circleBounds);

    // Number text
    g.setColour(stepNumberText);
    g.setFont(getSystemFont(15.0f, true));
    g.drawText(juce::String(stepNumber), circleBounds.toNearestInt(), juce::Justification::centred);

    // Step text
    stepBounds.removeFromLeft(20); // Gap
    g.setColour(textColor);
    g.setFont(getSystemFont(24.0f));
    g.drawFittedText(text, stepBounds, juce::Justification::centredLeft, 2);
}

void QuickStartGuide::drawStepWithSubitems(juce::Graphics& g, juce::Rectangle<int>& bounds,
                                            int stepNumber, const juce::String& intro,
                                            const std::vector<std::pair<juce::String, juce::String>>& items)
{
    // Calculate total height needed
    int totalHeight = 52 + (static_cast<int>(items.size()) * 42);
    auto stepBounds = bounds.removeFromTop(totalHeight);

    // Number circle
    auto numberArea = stepBounds.removeFromLeft(kStepNumberSize);
    auto circleBounds = numberArea.removeFromTop(kStepNumberSize).toFloat();
    g.setColour(stepNumberBg);
    g.fillEllipse(circleBounds);

    // Number text
    g.setColour(stepNumberText);
    g.setFont(getSystemFont(15.0f, true));
    g.drawText(juce::String(stepNumber), circleBounds.toNearestInt(), juce::Justification::centred);

    // Content area
    stepBounds.removeFromLeft(20); // Gap

    // Intro text
    auto introBounds = stepBounds.removeFromTop(48);
    g.setColour(textColor);
    g.setFont(getSystemFont(20.0f));
    g.drawText(intro, introBounds, juce::Justification::centredLeft);

    // Sub-items
    for (const auto& item : items)
    {
        auto itemBounds = stepBounds.removeFromTop(42);
        itemBounds.removeFromLeft(16); // Indent

        // Bullet
        g.setColour(textColor.withAlpha(0.7f));
        g.setFont(getSystemFont(20.0f));
        g.drawText(juce::String::charToString(0x2022), itemBounds.removeFromLeft(24), juce::Justification::centred);

        // Bold label
        g.setColour(textColor);
        g.setFont(getSystemFont(19.0f, true));
        auto labelFont = getSystemFont(19.0f, true);
        auto labelWidth = labelFont.getStringWidth(item.first) + 10;
        g.drawText(item.first, itemBounds.removeFromLeft(static_cast<int>(labelWidth)), juce::Justification::centredLeft);

        // Description
        g.setFont(getSystemFont(19.0f));
        g.setColour(textColor.withAlpha(0.8f));
        g.drawText("- " + item.second, itemBounds, juce::Justification::centredLeft);
    }
}

void QuickStartGuide::drawTipBox(juce::Graphics& g, juce::Rectangle<int>& bounds)
{
    auto tipBounds = bounds.removeFromTop(120);

    // Tip background
    g.setColour(tipBgColor);
    g.fillRoundedRectangle(tipBounds.toFloat(), 12.0f);

    // Left accent border
    g.setColour(tipBorderColor);
    g.fillRoundedRectangle(tipBounds.removeFromLeft(6).toFloat(), 12.0f);

    auto contentBounds = tipBounds.reduced(20, 16);

    // Tip label
    g.setColour(juce::Colour(0xFFFBBF24)); // Yellow
    g.setFont(getSystemFont(19.0f, true));
    g.drawText(LOCALIZE(GuideTip), contentBounds.removeFromTop(30), juce::Justification::centredLeft);

    // Tip text - two lines
    g.setColour(textColor);
    g.setFont(getSystemFont(18.0f));
    g.drawText(LOCALIZE(GuideTipText1),
               contentBounds.removeFromTop(26), juce::Justification::centredLeft);
    g.drawText(LOCALIZE(GuideTipText2),
               contentBounds, juce::Justification::centredLeft);
}

void QuickStartGuide::drawKeyboardShortcuts(juce::Graphics& g, juce::Rectangle<int>& bounds)
{
    auto shortcutsBounds = bounds.removeFromTop(160);

    // Background
    g.setColour(tipBgColor);
    g.fillRoundedRectangle(shortcutsBounds.toFloat(), 12.0f);

    // Left accent border (cyan/teal color for keyboard shortcuts)
    g.setColour(juce::Colour(0xFF22D3EE));
    g.fillRoundedRectangle(shortcutsBounds.removeFromLeft(6).toFloat(), 12.0f);

    auto contentBounds = shortcutsBounds.reduced(20, 16);

    // Title
    g.setColour(juce::Colour(0xFF22D3EE)); // Cyan
    g.setFont(getSystemFont(19.0f, true));
    g.drawText(LOCALIZE(GuideKeyboardShortcuts), contentBounds.removeFromTop(30), juce::Justification::centredLeft);

    // Shortcuts list
    g.setColour(textColor);
    g.setFont(getSystemFont(17.0f));

    auto drawShortcut = [&](const juce::String& keys, const juce::String& desc)
    {
        auto lineBounds = contentBounds.removeFromTop(26);
        auto keyBounds = lineBounds.removeFromLeft(180);

        // Keys in bold
        g.setFont(getSystemFont(17.0f, true));
        g.setColour(juce::Colour(0xFFE2E8F0)); // Lighter text for keys
        g.drawText(keys, keyBounds, juce::Justification::centredLeft);

        // Description
        g.setFont(getSystemFont(17.0f));
        g.setColour(textColor.withAlpha(0.85f));
        g.drawText(desc, lineBounds, juce::Justification::centredLeft);
    };

    drawShortcut("1-8", LOCALIZE(GuideShortcutSelectCard));
    drawShortcut("Tab / Shift+Tab", LOCALIZE(GuideShortcutNextPrev));
    drawShortcut("[ / ]", LOCALIZE(GuideShortcutBrackets));
    drawShortcut(juce::String::charToString(0x2190) + " " + juce::String::charToString(0x2191) + " " +
                 juce::String::charToString(0x2192) + " " + juce::String::charToString(0x2193),
                 LOCALIZE(GuideShortcutArrows));
}

} // namespace BlindCard
