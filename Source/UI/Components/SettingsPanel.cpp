/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "SettingsPanel.h"
#include "../Theme/FontManager.h"
#include <juce_audio_plugin_client/juce_audio_plugin_client.h>

namespace BlindCard
{
namespace
{
    const Language buttonLanguages[] = {
        Language::English, Language::Spanish, Language::TraditionalChinese,
        Language::SimplifiedChinese, Language::Japanese, Language::Korean
    };


    juce::Font displayFont(float height)
    {
        auto& fonts = FontManager::getInstance();
        return LocalizationManager::getInstance().getCurrentLanguage() == Language::English
            ? fonts.getMuseoModernoBold(height) : fonts.getBold(height);
    }

    float labelWidth(const juce::Font& font, const juce::String& text)
    {
        juce::GlyphArrangement glyphs;
        glyphs.addLineOfText(font, text, 0.0f, 0.0f);
        return glyphs.getBoundingBox(0, glyphs.getNumGlyphs(), true).getWidth();
    }

    void drawProductMark(juce::Graphics& g, juce::Rectangle<float> bounds, float fontHeight)
    {
        const auto tile = bounds.withSizeKeepingCentre(fontHeight - 4.0f, fontHeight - 4.0f)
                                .withX(bounds.getX());
        g.setColour(juce::Colour(0xFF8B2635));
        g.fillRoundedRectangle(tile, 5.0f);
        const auto inner = tile.reduced(tile.getWidth() * 0.19f);
        auto x = [&](float value) { return inner.getX() + inner.getWidth() * value; };
        auto y = [&](float value) { return inner.getY() + inner.getHeight() * value; };
        juce::Path spade;
        spade.startNewSubPath(x(0.5f), y(0));
        spade.cubicTo(x(0.5f), y(0), x(0), y(0.4f), x(0), y(0.6f));
        spade.cubicTo(x(0), y(0.8f), x(0.25f), y(0.85f), x(0.5f), y(0.7f));
        spade.cubicTo(x(0.75f), y(0.85f), x(1), y(0.8f), x(1), y(0.6f));
        spade.cubicTo(x(1), y(0.4f), x(0.5f), y(0), x(0.5f), y(0));
        spade.closeSubPath();
        spade.startNewSubPath(x(0.45f), y(0.65f));
        spade.lineTo(x(0.38f), y(1));
        spade.lineTo(x(0.62f), y(1));
        spade.lineTo(x(0.55f), y(0.65f));
        spade.closeSubPath();
        g.setColour(juce::Colour(0xFFFAF7F2));
        g.fillPath(spade);

        auto font = FontManager::getInstance().getMuseoModernoBold(fontHeight)
                                                .withExtraKerningFactor(-0.01f);
        const float textX = tile.getRight() + 12.0f;
        const float blindWidth = labelWidth(font, "Blind") + 1.0f;
        g.setFont(font);
        g.setColour(ThemeManager::getInstance().isDark()
            ? juce::Colour(0xFFEAF1ED) : juce::Colour(0xFF10302C));
        g.drawText("Blind", juce::Rectangle<float>{ textX, bounds.getY(), blindWidth, bounds.getHeight() },
                   juce::Justification::centredLeft);
        g.setColour(ThemeManager::getInstance().isDark()
            ? juce::Colour(0xFFF17B80) : juce::Colour(0xFFBA3038));
        g.drawText("Card", juce::Rectangle<float>{ textX + blindWidth, bounds.getY(),
                            labelWidth(font, "Card") + 2.0f, bounds.getHeight() },
                   juce::Justification::centredLeft);
    }

    void drawClose(juce::Graphics& g, juce::Rectangle<int> bounds, juce::Colour colour)
    {
        const auto r = bounds.toFloat().reduced(10.0f);
        g.setColour(colour);
        g.drawLine(r.getX(), r.getY(), r.getRight(), r.getBottom(), 1.8f);
        g.drawLine(r.getRight(), r.getY(), r.getX(), r.getBottom(), 1.8f);
    }
}

class SettingsPanel::LanguageLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour&, bool over, bool down) override
    {
        const bool selected = button.getToggleState();
        const auto bounds = button.getLocalBounds().toFloat().reduced(3.0f, 0.0f);
        const auto ink = button.findColour(juce::TextButton::textColourOnId);
        if (over || down)
        {
            g.setColour(ink.withAlpha(down ? 0.13f : 0.07f));
            g.fillRect(bounds);
        }
        if (selected)
        {
            g.setColour(ink);
            g.fillRect(bounds.getX() + 5.0f, bounds.getBottom() - 2.0f,
                       bounds.getWidth() - 10.0f, 2.0f);
        }
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool over, bool down) override
    {
        g.setColour(button.findColour(button.getToggleState() || over || down
            ? juce::TextButton::textColourOnId : juce::TextButton::textColourOffId));
        g.setFont(FontManager::getInstance().getMedium(15.0f));
        g.drawText(button.getButtonText(), button.getLocalBounds().reduced(4, 0),
                   juce::Justification::centred);
    }
};

SettingsPanel::SettingsPanel()
{
    setOpaque(false);
    setAlwaysOnTop(true);
    setInterceptsMouseClicks(true, true);
    versionString = JucePlugin_VersionString;
    languageLookAndFeel = std::make_unique<LanguageLookAndFeel>();
    setupLanguageSelector();
    createSvgAccents();
    updatePalette();
    LocalizationManager::getInstance().addListener(this);
    ThemeManager::getInstance().addChangeListener(this);
}

SettingsPanel::~SettingsPanel()
{
    ThemeManager::getInstance().removeChangeListener(this);
    LocalizationManager::getInstance().removeListener(this);
    for (auto* button : languageButtons)
        button->setLookAndFeel(nullptr);
}

void SettingsPanel::createSvgAccents()
{
    // Original single-ink line art. Parse once, then recolour the cached Drawables on theme changes.
    const std::array<const char*, static_cast<size_t>(AccentSvg::Count)> artwork {{
        R"svg(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 132 88">
          <g fill="none" stroke="#000000" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round">
            <g transform="rotate(-14 47 44)"><rect x="24" y="12" width="46" height="64" rx="4"/>
              <path d="M30 23v-5h5M59 70h5v-5"/></g>
            <rect x="61" y="12" width="46" height="64" rx="4"/>
            <path d="M67 23v-5h5M96 70h5v-5M84 30c-3 5-12 9-12 15 0 5 7 7 12 2 5 5 12 3 12-2 0-6-9-10-12-15zM82 48l-2 9h8l-2-9"/>
          </g></svg>)svg",
        R"svg(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><g fill="none" stroke="#000000" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round"><path d="M3 5h12M9 3v2M13 5c-1 6-5 10-10 12M5 8c2 4 5 7 9 9M14 21l4-11 4 11M15.5 17h5"/></g></svg>)svg",
        R"svg(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><g fill="none" stroke="#000000" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round"><path d="M4 4h9l7 7a2 2 0 0 1 0 3l-6 6a2 2 0 0 1-3 0l-7-7z"/><circle cx="8" cy="8" r="1.5"/></g></svg>)svg",
        R"svg(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><g fill="none" stroke="#000000" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="7" r="3.5"/><path d="M5 21v-3a7 7 0 0 1 14 0v3M9 21v-3M15 21v-3"/></g></svg>)svg",
        R"svg(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><g fill="none" stroke="#000000" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="9"/><ellipse cx="12" cy="12" rx="4" ry="9"/><path d="M3 12h18"/></g></svg>)svg",
        R"svg(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><g fill="none" stroke="#000000" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round"><path d="M12 5v15M3 4c4-1 6-1 9 1 3-2 5-2 9-1v15c-4-1-6-1-9 1-3-2-5-2-9-1zM6 8h3M15 8h3"/></g></svg>)svg",
        R"svg(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><g fill="none" stroke="#000000" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round"><path d="M12 3v12M7 10l5 5 5-5M4 16v5h16v-5"/></g></svg>)svg"
    }};
    for (size_t i = 0; i < artwork.size(); ++i)
        if (auto xml = juce::parseXML(artwork[i]))
            svgAccents[i] = juce::Drawable::createFromSVG(*xml);
}

void SettingsPanel::drawSvgAccent(juce::Graphics& g, AccentSvg icon,
                                   juce::Rectangle<float> bounds, float opacity) const
{
    if (auto* drawable = svgAccents[static_cast<size_t>(icon)].get())
        drawable->drawWithin(g, bounds, juce::RectanglePlacement::centred, opacity);
}

void SettingsPanel::drawRowIcon(juce::Graphics& g, AccentSvg icon, juce::Rectangle<int> row) const
{
    drawSvgAccent(g, icon, juce::Rectangle<float>(static_cast<float>(row.getX()),
                  static_cast<float>(row.getCentreY()) - 7.0f, 14.0f, 14.0f), 0.9f);
}

void SettingsPanel::setUpdateInfo(bool available, const juce::String& latestVer,
                                  const juce::String& url, const juce::String& changelog)
{
    updateAvailable = available;
    latestVersionString = latestVer;
    downloadUrl = url;
    changelogText = changelog;
    resized();
    repaint();
}

void SettingsPanel::setupLanguageSelector()
{
    const char* names[] = { "EN", "ES", "繁", "简", "日", "한" };
    for (int i = 0; i < 6; ++i)
    {
        auto* button = languageButtons.add(new juce::TextButton(juce::String::fromUTF8(names[i])));
        button->setLookAndFeel(languageLookAndFeel.get());
        button->setClickingTogglesState(false);
        button->onClick = [this, i] { onLanguageSelected(i); };
        addAndMakeVisible(button);
    }
}

void SettingsPanel::onLanguageSelected(int index)
{
    if (index >= 0 && index < 6)
        LocalizationManager::getInstance().setLanguage(buttonLanguages[index]);
    updateLanguageButtonStates();
}

void SettingsPanel::updateLanguageButtonStates()
{
    const auto language = LocalizationManager::getInstance().getCurrentLanguage();
    for (int i = 0; i < languageButtons.size(); ++i)
    {
        auto* button = languageButtons[i];
        button->setToggleState(language == buttonLanguages[i], juce::dontSendNotification);
        button->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        button->setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
        button->setColour(juce::TextButton::textColourOffId, mutedColor);
        button->setColour(juce::TextButton::textColourOnId, titleColor);
        button->repaint();
    }
}

void SettingsPanel::updatePalette()
{
    const bool dark = ThemeManager::getInstance().isDark();
    // BlindCard product-page sage and dark teal, adapted for native text contrast.
    bgColor = juce::Colour(dark ? 0xFF343A37 : 0xFFAEBFBC);
    borderColor = juce::Colour(dark ? 0xFF768C80 : 0xFF7F9993);
    titleColor = juce::Colour(dark ? 0xFFEAF1ED : 0xFF10302C);
    textColor = juce::Colour(dark ? 0xFFD4DFD9 : 0xFF183D36);
    mutedColor = juce::Colour(dark ? 0xFFC1D0C7 : 0xFF2F4D48);
    accentColor = juce::Colour(dark ? 0xFFB9D0C2 : 0xFF10302C);
    linkColor = juce::Colour(dark ? 0xFFD8E6DD : 0xFF10302C);
    for (auto& drawable : svgAccents)
        if (drawable != nullptr)
            drawable->replaceColour(svgAccentInk, mutedColor);
    svgAccentInk = mutedColor;
    updateLanguageButtonStates();
}

void SettingsPanel::changeListenerCallback(juce::ChangeBroadcaster*)
{
    updatePalette();
    repaint();
}

void SettingsPanel::languageChanged()
{
    updateLanguageButtonStates();
    resized();
    repaint();
    if (onLanguageChanged) onLanguageChanged();
}

void SettingsPanel::showOverlay(juce::Component* parent)
{
    if (parent == nullptr) return;
    updatePalette();
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

void SettingsPanel::drawExternalLinkIcon(juce::Graphics& g,
                                         const juce::Rectangle<int>& linkBounds) const
{
    auto r = linkBounds.withTrimmedLeft(linkBounds.getWidth() - 16).toFloat().withSizeKeepingCentre(10.0f, 10.0f);
    g.setColour(linkColor);
    g.drawLine(r.getX(), r.getBottom(), r.getRight(), r.getY(), 1.4f);
    g.drawLine(r.getX() + 3.0f, r.getY(), r.getRight(), r.getY(), 1.4f);
    g.drawLine(r.getRight(), r.getY(), r.getRight(), r.getBottom() - 3.0f, 1.4f);
}

void SettingsPanel::paint(juce::Graphics& g)
{
    auto& fonts = FontManager::getInstance();
    g.fillAll(juce::Colours::black.withAlpha(0.46f));
    g.setColour(bgColor);
    g.fillRoundedRectangle(dialogBounds.toFloat(), kCornerRadius);
    g.setColour(borderColor);
    g.drawRoundedRectangle(dialogBounds.toFloat().reduced(0.5f), kCornerRadius, 1.0f);

    static const juce::Image sugoiLogo = juce::ImageCache::getFromMemory(
        BinaryData::sugoilogoheader_png, BinaryData::sugoilogoheader_pngSize);
    const juce::Rectangle<float> logoBounds(static_cast<float>(dialogBounds.getX() + kPadding),
                                             static_cast<float>(dialogBounds.getY() + 18), 30.0f, 30.0f);
    if (sugoiLogo.isValid())
        g.drawImage(sugoiLogo, logoBounds, juce::RectanglePlacement::centred);
    g.setColour(titleColor);
    g.setFont(fonts.getMuseoModernoBold(24.0f).withExtraKerningFactor(0.01f));
    g.drawText("Sugoi", logoBounds.withX(logoBounds.getRight() + 8.0f).withWidth(126.0f),
               juce::Justification::centredLeft);
    drawProductMark(g, { static_cast<float>(dialogBounds.getX() + kPadding),
                        static_cast<float>(dialogBounds.getY() + 52),
                        static_cast<float>(dialogBounds.getWidth() - 2 * kPadding - 40), 44.0f }, 38.0f);
    g.setColour(titleColor);
    g.setFont(displayFont(18.0f));
    g.drawText(LOCALIZE(AboutTitle), dialogBounds.withTrimmedTop(101).withHeight(27).reduced(kPadding, 0),
               juce::Justification::centredLeft);
    drawClose(g, closeButtonBounds, mutedColor);
    drawSvgAccent(g, AccentSvg::Header,
                  { static_cast<float>(dialogBounds.getRight() - 164),
                    static_cast<float>(dialogBounds.getY() + 57), 132.0f, 78.0f }, 0.32f);

    auto notice = noticeBounds;
    g.setFont(fonts.getMedium(16.0f));
    g.setColour(titleColor);
    g.drawText(LOCALIZE(AboutFreeSoftware), notice.removeFromTop(22), juce::Justification::centredLeft);
    g.setFont(fonts.getRegular(14.5f));
    g.setColour(mutedColor);
    g.drawText(LOCALIZE(AboutNoLicense), notice, juce::Justification::centredLeft);

    g.setFont(fonts.getMedium(14.0f));
    g.setColour(mutedColor);
    drawRowIcon(g, AccentSvg::Language, languageLabelBounds);
    g.drawText(LOCALIZE(SettingsLanguage), languageLabelBounds.withTrimmedLeft(kIconTextInset),
               juce::Justification::centredLeft);
    g.setColour(borderColor);
    g.fillRect(dialogBounds.getX() + kPadding, infoSeparatorY,
               dialogBounds.getWidth() - 2 * kPadding, 1);

    auto drawInfoRow = [&](juce::Rectangle<int> row, const juce::String& label,
                           const juce::String& value, AccentSvg icon)
    {
        drawRowIcon(g, icon, row);
        g.setFont(fonts.getRegular(15.0f));
        g.setColour(mutedColor);
        g.drawText(label, row.removeFromLeft(150).withTrimmedLeft(kIconTextInset), juce::Justification::centredLeft);
        g.setColour(titleColor);
        g.setFont(fonts.getMedium(15.0f));
        g.drawText(value, row, juce::Justification::centredRight);
    };
    drawInfoRow(versionRowBounds, LOCALIZE(AboutVersion), versionString, AccentSvg::Version);
    drawInfoRow(developerRowBounds, LOCALIZE(AboutDeveloper), developerName, AccentSvg::Developer);

    auto status = updateRowBounds;
    g.setFont(fonts.getRegular(14.0f));
    g.setColour(mutedColor);
    if (updateAvailable && latestVersionString.isNotEmpty())
    {
        status.removeFromRight(updateLinkBounds.getWidth() + 8);
        g.drawText(LOCALIZE(UpdateAvailable) + " v" + latestVersionString,
                   status, juce::Justification::centredLeft);
        g.setColour(linkColor);
        g.setFont(fonts.getMedium(14.0f));
        g.drawText(LOCALIZE(UpdateDownload), updateLinkBounds, juce::Justification::centredRight);
    }
    else
        g.drawText(LOCALIZE(UpdateUpToDate), status, juce::Justification::centredRight);

    if (!changelogBounds.isEmpty())
    {
        g.setColour(mutedColor);
        g.setFont(fonts.getRegular(13.0f));
        g.drawFittedText(changelogText, changelogBounds, juce::Justification::topLeft, 2, 1.0f);
    }

    auto drawLink = [&](juce::Rectangle<int> row, const juce::String& label, AccentSvg icon)
    {
        drawRowIcon(g, icon, row);
        g.setColour(linkColor);
        g.setFont(fonts.getMedium(15.0f));
        g.drawText(label, row.withTrimmedRight(24).withTrimmedLeft(kIconTextInset), juce::Justification::centredLeft);
        drawExternalLinkIcon(g, row);
    };
    drawLink(websiteLinkBounds, LOCALIZE(AboutVisitWebsite), AccentSvg::Website);
    drawLink(manualLinkBounds, LOCALIZE(ManualPage), AccentSvg::Manual);
    drawLink(updatePageLinkBounds, LOCALIZE(UpdatePage), AccentSvg::Update);
}

void SettingsPanel::resized()
{
    if (getWidth() <= 0 || getHeight() <= 0) return;
    const bool showChangelog = updateAvailable && changelogText.isNotEmpty();
    const int width = juce::jmin(kDialogWidth, juce::jmax(1, getWidth() - 32));
    const int height = juce::jmin(484 + (showChangelog ? 36 : 0), juce::jmax(1, getHeight() - 32));
    dialogBounds = getLocalBounds().withSizeKeepingCentre(width, height);
    closeButtonBounds = { dialogBounds.getRight() - kPadding - 32,
                          dialogBounds.getY() + 20, 32, 32 };
    auto rows = dialogBounds.reduced(kPadding, 0);
    rows.removeFromTop(kHeaderHeight + 16);
    noticeBounds = rows.removeFromTop(42);
    rows.removeFromTop(20);
    auto languageRow = rows.removeFromTop(32);
    languageLabelBounds = languageRow.removeFromLeft(92);
    const int buttonWidth = languageRow.getWidth() / 6;
    for (int i = 0; i < languageButtons.size(); ++i)
        languageButtons[i]->setBounds(languageRow.getX() + i * buttonWidth,
                                      languageRow.getY(), buttonWidth, languageRow.getHeight());
    rows.removeFromTop(20);
    infoSeparatorY = rows.getY();
    rows.removeFromTop(13);
    versionRowBounds = rows.removeFromTop(24);
    updateRowBounds = rows.removeFromTop(24);
    updateLinkBounds = updateAvailable && latestVersionString.isNotEmpty()
        ? updateRowBounds.withTrimmedLeft(updateRowBounds.getWidth() - 90)
        : juce::Rectangle<int>();
    changelogBounds = showChangelog ? rows.removeFromTop(36) : juce::Rectangle<int>();
    developerRowBounds = rows.removeFromTop(24);
    rows.removeFromTop(20);
    websiteLinkBounds = rows.removeFromTop(28);
    manualLinkBounds = rows.removeFromTop(28);
    updatePageLinkBounds = rows.removeFromTop(28);
}

void SettingsPanel::mouseDown(const juce::MouseEvent& event)
{
    const auto pos = event.getPosition();
    if (closeButtonBounds.contains(pos) || !dialogBounds.contains(pos))
    {
        hideOverlay();
        if (onClose) onClose();
        return;
    }
    if (!updateLinkBounds.isEmpty() && updateLinkBounds.contains(pos))
    {
        if (downloadUrl.isNotEmpty()) juce::URL(downloadUrl).launchInDefaultBrowser();
        return;
    }
    if (websiteLinkBounds.contains(pos)) juce::URL(websiteUrl).launchInDefaultBrowser();
    else if (manualLinkBounds.contains(pos)) juce::URL(manualUrl).launchInDefaultBrowser();
    else if (updatePageLinkBounds.contains(pos)) juce::URL(updatePageUrl).launchInDefaultBrowser();
}

} // namespace BlindCard
