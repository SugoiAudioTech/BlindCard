/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include "../Localization/LocalizationManager.h"
#include "../Theme/ThemeManager.h"

namespace BlindCard
{

/** Theme-aware information and language overlay. */
class SettingsPanel : public juce::Component,
                      public LocalizationManager::Listener,
                      private juce::ChangeListener
{
public:
    SettingsPanel();
    ~SettingsPanel() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    void languageChanged() override;

    std::function<void()> onClose;
    std::function<void()> onLanguageChanged;
    void setUpdateInfo(bool available, const juce::String& latestVer, const juce::String& url,
                       const juce::String& changelog = {});
    void showOverlay(juce::Component* parent);
    void hideOverlay();

private:
    static constexpr int kDialogWidth = 480;
    static constexpr int kCornerRadius = 8;
    static constexpr int kPadding = 28;
    static constexpr int kHeaderHeight = 140;

    juce::Colour bgColor, borderColor, titleColor, textColor, mutedColor;
    juce::Colour accentColor, linkColor;
    enum class AccentSvg { Header, Language, Version, Developer, Website, Manual, Update, Count };
    std::array<std::unique_ptr<juce::Drawable>, static_cast<size_t>(AccentSvg::Count)> svgAccents;
    juce::Colour svgAccentInk = juce::Colours::black;
    static constexpr int kIconTextInset = 22;
    void createSvgAccents();
    void drawSvgAccent(juce::Graphics&, AccentSvg, juce::Rectangle<float>, float opacity) const;
    void drawRowIcon(juce::Graphics&, AccentSvg, juce::Rectangle<int>) const;
    juce::Rectangle<int> dialogBounds, closeButtonBounds, noticeBounds;
    juce::Rectangle<int> languageLabelBounds, versionRowBounds, updateRowBounds;
    juce::Rectangle<int> changelogBounds, developerRowBounds;
    juce::Rectangle<int> websiteLinkBounds, updateLinkBounds, updatePageLinkBounds, manualLinkBounds;
    int infoSeparatorY = 0;

    juce::String versionString;
    juce::String developerName = "Sugoi Audio";
    juce::String websiteUrl = "https://sugoiaudio.com";
    juce::String updatePageUrl = "https://sugoiaudio.com/downloads/blindcard";
    juce::String manualUrl = "https://sugoiaudio.com/products/blindcard-manual.html";
    bool updateAvailable = false;
    juce::String latestVersionString, downloadUrl, changelogText;

    // Native child buttons avoid a separate ComboBox popup window in AU hosts.
    class LanguageLookAndFeel;
    std::unique_ptr<LanguageLookAndFeel> languageLookAndFeel;
    juce::OwnedArray<juce::TextButton> languageButtons;
    void setupLanguageSelector();
    void onLanguageSelected(int);
    void updateLanguageButtonStates();
    void updatePalette();
    void changeListenerCallback(juce::ChangeBroadcaster*) override;
    void drawExternalLinkIcon(juce::Graphics&, const juce::Rectangle<int>&) const;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsPanel)
};

} // namespace BlindCard
