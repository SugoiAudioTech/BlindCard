/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Theme/ThemeManager.h"
#include "../Localization/LocalizationManager.h"

namespace BlindCard
{

/** A compact, theme-aware guide with measured scrollable content. */
class QuickStartGuide : public juce::Component,
                        public LocalizationManager::Listener,
                        private juce::ChangeListener
{
public:
    QuickStartGuide();
    ~QuickStartGuide() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    void languageChanged() override;
    std::function<void()> onClose;
    void showOverlay(juce::Component* parent);
    void hideOverlay();

private:
    static constexpr int kDialogWidth = 680;
    static constexpr int kDialogHeight = 678;
    static constexpr int kCornerRadius = 8;
    static constexpr int kPadding = 28;
    static constexpr int kHeaderHeight = 120;
    static constexpr int kFooterHeight = 60;
    static constexpr int kButtonHeight = 36;
    static constexpr int kButtonWidth = 156;

    juce::Colour bgColor, borderColor, titleColor, textColor, mutedColor;
    juce::Colour accentColor, numberTextColor;
    juce::Rectangle<int> dialogBounds, closeButtonBounds, gotItButtonBounds, scrollAreaBounds;

    class ContentComponent : public juce::Component
    {
    public:
        explicit ContentComponent(QuickStartGuide& owner) : parent(owner) {}
        void paint(juce::Graphics& g) override { parent.layoutContent(&g); }
    private:
        QuickStartGuide& parent;
    };
    std::unique_ptr<ContentComponent> contentComponent;
    std::unique_ptr<juce::Viewport> viewport;
    int layoutContent(juce::Graphics* graphics) const;
    void updatePalette();
    void changeListenerCallback(juce::ChangeBroadcaster*) override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuickStartGuide)
};

} // namespace BlindCard
