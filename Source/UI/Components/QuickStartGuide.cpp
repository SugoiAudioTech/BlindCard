/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "QuickStartGuide.h"
#include "../Theme/FontManager.h"

#include <array>
#include <cmath>

namespace BlindCard
{
namespace
{
    float wrappedText(juce::Graphics* graphics, const juce::String& text,
                      const juce::Font& font, juce::Colour colour,
                      float x, float y, float width)
    {
        juce::AttributedString attributed;
        attributed.setWordWrap(juce::AttributedString::WordWrap::byWord);
        attributed.setLineSpacing(3.0f);
        attributed.append(text, font, colour);
        juce::TextLayout layout;
        layout.createLayout(attributed, juce::jmax(1.0f, width));
        const float height = std::ceil(layout.getHeight());
        if (graphics != nullptr)
            layout.draw(*graphics, { x, y, width, height });
        return height;
    }


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

    void closeIcon(juce::Graphics& g, juce::Rectangle<int> bounds, juce::Colour colour)
    {
        const auto r = bounds.toFloat().reduced(10.0f);
        g.setColour(colour);
        g.drawLine(r.getX(), r.getY(), r.getRight(), r.getBottom(), 1.8f);
        g.drawLine(r.getRight(), r.getY(), r.getX(), r.getBottom(), 1.8f);
    }
}

QuickStartGuide::QuickStartGuide()
{
    setOpaque(false);
    setAlwaysOnTop(true);
    setInterceptsMouseClicks(true, true);
    contentComponent = std::make_unique<ContentComponent>(*this);
    contentComponent->setOpaque(false);
    viewport = std::make_unique<juce::Viewport>();
    viewport->setViewedComponent(contentComponent.get(), false);
    viewport->setScrollBarsShown(true, false);
    viewport->setScrollBarThickness(8);
    viewport->setScrollOnDragMode(juce::Viewport::ScrollOnDragMode::all);
    viewport->getVerticalScrollBar().setAutoHide(true);
    addAndMakeVisible(viewport.get());
    updatePalette();
    LocalizationManager::getInstance().addListener(this);
    ThemeManager::getInstance().addChangeListener(this);
}

QuickStartGuide::~QuickStartGuide()
{
    ThemeManager::getInstance().removeChangeListener(this);
    LocalizationManager::getInstance().removeListener(this);
    viewport->setViewedComponent(nullptr, false);
}

void QuickStartGuide::updatePalette()
{
    const bool dark = ThemeManager::getInstance().isDark();
    // Reading surface and ink follow the BlindCard manual's sage / teal pairing.
    bgColor = juce::Colour(dark ? 0xFF343A37 : 0xFFAEBFBC);
    borderColor = juce::Colour(dark ? 0xFF768C80 : 0xFF8CA29B);
    titleColor = juce::Colour(dark ? 0xFFEAF1ED : 0xFF10302C);
    textColor = juce::Colour(dark ? 0xFFD4DFD9 : 0xFF183D36);
    mutedColor = juce::Colour(dark ? 0xFFC1D0C7 : 0xFF2F4D48);
    accentColor = juce::Colour(dark ? 0xFFB9D0C2 : 0xFF10302C);
    numberTextColor = juce::Colour(dark ? 0xFFB9D0C2 : 0xFF2F4D48);
    viewport->getVerticalScrollBar().setColour(juce::ScrollBar::thumbColourId,
        juce::Colour(dark ? 0xFF929A90 : 0xFF4E6A62));
}

void QuickStartGuide::changeListenerCallback(juce::ChangeBroadcaster*)
{
    updatePalette();
    repaint();
    contentComponent->repaint();
}

void QuickStartGuide::languageChanged()
{
    resized();
    repaint();
    contentComponent->repaint();
}

void QuickStartGuide::showOverlay(juce::Component* parent)
{
    if (parent == nullptr) return;
    updatePalette();
    setBounds(parent->getLocalBounds());
    parent->addAndMakeVisible(this);
    toFront(true);
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
    auto& fonts = FontManager::getInstance();
    g.fillAll(juce::Colours::black.withAlpha(0.46f));
    g.setColour(bgColor);
    g.fillRoundedRectangle(dialogBounds.toFloat(), kCornerRadius);
    g.setColour(borderColor);
    g.drawRoundedRectangle(dialogBounds.toFloat().reduced(0.5f), kCornerRadius, 1.0f);

    drawProductMark(g, { static_cast<float>(dialogBounds.getX() + kPadding),
                        static_cast<float>(dialogBounds.getY() + 18),
                        static_cast<float>(dialogBounds.getWidth() - 2 * kPadding - 40), 40.0f }, 34.0f);
    auto heading = dialogBounds.withTrimmedTop(74).withHeight(30).reduced(kPadding, 0);
    g.setColour(titleColor);
    g.setFont(displayFont(24.0f));
    g.drawText(LOCALIZE(GuideTitle), heading, juce::Justification::centredLeft);
    closeIcon(g, closeButtonBounds, mutedColor);

    g.setColour(borderColor);
    g.fillRect(dialogBounds.getX() + kPadding, dialogBounds.getY() + kHeaderHeight,
               dialogBounds.getWidth() - kPadding * 2, 1);
    g.fillRect(dialogBounds.getX() + kPadding, dialogBounds.getBottom() - kFooterHeight,
               dialogBounds.getWidth() - kPadding * 2, 1);

    g.setColour(accentColor);
    g.fillRoundedRectangle(gotItButtonBounds.toFloat(), 4.0f);
    g.setColour(ThemeManager::getInstance().isDark() ? juce::Colour(0xFF10302C)
                                                 : juce::Colour(0xFFEAF1ED));
    g.setFont(fonts.getMedium(16.0f));
    g.drawText(LOCALIZE(GuideGotIt), gotItButtonBounds.reduced(12, 0), juce::Justification::centred);
}

void QuickStartGuide::resized()
{
    if (getWidth() <= 0 || getHeight() <= 0) return;
    const int width = juce::jmin(kDialogWidth, juce::jmax(1, getWidth() - 32));
    const int height = juce::jmin(kDialogHeight, juce::jmax(1, getHeight() - 32));
    dialogBounds = getLocalBounds().withSizeKeepingCentre(width, height);
    closeButtonBounds = { dialogBounds.getRight() - kPadding - 32,
                          dialogBounds.getY() + 20, 32, 32 };
    gotItButtonBounds = { dialogBounds.getRight() - kPadding - kButtonWidth,
                          dialogBounds.getBottom() - kFooterHeight + (kFooterHeight - kButtonHeight) / 2,
                          kButtonWidth, kButtonHeight };
    scrollAreaBounds = { dialogBounds.getX() + kPadding, dialogBounds.getY() + kHeaderHeight + 1,
                         dialogBounds.getWidth() - 2 * kPadding,
                         dialogBounds.getHeight() - kHeaderHeight - kFooterHeight - 2 };
    viewport->setBounds(scrollAreaBounds);
    const int contentWidth = juce::jmax(1, scrollAreaBounds.getWidth() - 16);
    contentComponent->setSize(contentWidth, 1);
    contentComponent->setSize(contentWidth, layoutContent(nullptr));
}

void QuickStartGuide::mouseDown(const juce::MouseEvent& event)
{
    const auto pos = event.getPosition();
    if (closeButtonBounds.contains(pos) || gotItButtonBounds.contains(pos)
        || !dialogBounds.contains(pos))
    {
        hideOverlay();
        if (onClose) onClose();
    }
}

int QuickStartGuide::layoutContent(juce::Graphics* graphics) const
{
    auto& fonts = FontManager::getInstance();
    const auto body = fonts.getRegular(16.0f);
    const auto medium = fonts.getMedium(16.0f);
    const auto small = fonts.getRegular(15.0f);
    const auto bold = displayFont(16.0f);
    const float width = static_cast<float>(contentComponent->getWidth());
    float y = 20.0f;

    auto text = [&](const juce::String& value, const juce::Font& font, juce::Colour colour,
                    float x, float top, float availableWidth)
    {
        return wrappedText(graphics, value, font, colour, x, top, availableWidth);
    };
    auto measure = [&](const juce::String& value, const juce::Font& font, float availableWidth)
    {
        return wrappedText(nullptr, value, font, textColor, 0, 0, availableWidth);
    };
    y += text(LOCALIZE(GuideDescription), body, mutedColor, 0, y, width);
    y += 20.0f;

    auto number = [&](int value, float top)
    {
        if (graphics == nullptr) return;
        const juce::Rectangle<float> bounds(0.0f, top - 1.0f, 32.0f, 28.0f);
        graphics->setColour(numberTextColor);
        graphics->setFont(fonts.getMuseoModernoBold(22.0f));
        graphics->drawText(juce::String(value).paddedLeft('0', 2), bounds,
                           juce::Justification::centredLeft);
    };
    auto step = [&](int value, const juce::String& description)
    {
        number(value, y);
        const float height = text(description, medium, textColor, 46.0f, y + 3.0f, width - 46.0f);
        y += juce::jmax(24.0f, height + 3.0f) + 14.0f;
    };

    step(1, LOCALIZE(GuideStep1Full));
    number(2, y);
    y += juce::jmax(24.0f, text(LOCALIZE(GuideStep2Intro), medium, textColor,
                              46.0f, y + 3.0f, width - 46.0f) + 3.0f) + 8.0f;
    const std::array<std::pair<juce::String, juce::String>, 3> modes {{
        { LOCALIZE(ModeStars), LOCALIZE(GuideModeStarsDesc) },
        { LOCALIZE(ModeGuess), LOCALIZE(GuideModeGuessDesc) },
        { LOCALIZE(ModeQA), LOCALIZE(GuideModeQADesc) }
    }};
    for (const auto& mode : modes)
    {
        const float labelHeight = text(mode.first, bold, titleColor, 46.0f, y, 74.0f);
        const float descriptionHeight = text(mode.second, small, mutedColor, 128.0f, y, width - 128.0f);
        y += juce::jmax(labelHeight, descriptionHeight) + 8.0f;
    }
    y += 8.0f;
    step(3, LOCALIZE(GuideStep3Full));
    step(4, LOCALIZE(GuideStep4Full));
    step(5, LOCALIZE(GuideStep5Full));
    step(6, LOCALIZE(GuideStep6Full));
    y += 6.0f;

    // Related tip lines form three paragraphs, preserving every localized sentence.
    const std::array<juce::String, 3> tips {{
        LOCALIZE(GuideTipText1) + " " + LOCALIZE(GuideTipText2),
        LOCALIZE(GuideTipText3) + " " + LOCALIZE(GuideTipText4),
        LOCALIZE(GuideTipText5) + " " + LOCALIZE(GuideTipText6)
    }};
    const float sectionInset = 0.0f;
    const float innerWidth = width - 2 * sectionInset;
    float tipsHeight = 14.0f + measure(LOCALIZE(GuideTip), bold, innerWidth) + 10.0f;
    for (const auto& paragraph : tips)
        tipsHeight += measure(paragraph, small, innerWidth) + 8.0f;
    tipsHeight += 6.0f;
    if (graphics != nullptr)
    {
        graphics->setColour(borderColor);
        graphics->fillRect(0.0f, y, width, 1.0f);
    }
    const float tipsTop = y;
    y += 14.0f;
    y += text(LOCALIZE(GuideTip), bold, titleColor, sectionInset, y, innerWidth) + 10.0f;
    for (const auto& paragraph : tips)
        y += text(paragraph, small, textColor, sectionInset, y, innerWidth) + 8.0f;
    y = tipsTop + tipsHeight + 14.0f;

    const std::array<std::pair<juce::String, juce::String>, 4> shortcuts {{
        { juce::String::fromUTF8("1–8"), LOCALIZE(GuideShortcutSelectCard) },
        { "Tab / Shift+Tab", LOCALIZE(GuideShortcutNextPrev) },
        { "[ / ]", LOCALIZE(GuideShortcutBrackets) },
        { juce::String::fromUTF8("← ↑ → ↓"), LOCALIZE(GuideShortcutArrows) }
    }};
    const float keyWidth = 126.0f;
    const float descriptionWidth = innerWidth - keyWidth - 12.0f;
    float shortcutHeight = 14.0f + measure(LOCALIZE(GuideKeyboardShortcuts), bold, innerWidth) + 12.0f;
    for (const auto& shortcut : shortcuts)
        shortcutHeight += juce::jmax(measure(shortcut.first, fonts.getMedium(15.0f), keyWidth),
                                     measure(shortcut.second, small, descriptionWidth)) + 9.0f;
    shortcutHeight += 5.0f;
    if (graphics != nullptr)
    {
        graphics->setColour(borderColor);
        graphics->fillRect(0.0f, y, width, 1.0f);
    }
    const float shortcutsTop = y;
    y += 14.0f;
    y += text(LOCALIZE(GuideKeyboardShortcuts), bold, titleColor, sectionInset, y, innerWidth) + 12.0f;
    for (const auto& shortcut : shortcuts)
    {
        const float keyHeight = text(shortcut.first, fonts.getMedium(15.0f), titleColor,
                                     sectionInset, y, keyWidth);
        const float descriptionHeight = text(shortcut.second, small, textColor,
                                             sectionInset + keyWidth + 12.0f, y, descriptionWidth);
        y += juce::jmax(keyHeight, descriptionHeight) + 9.0f;
    }
    return static_cast<int>(std::ceil(shortcutsTop + shortcutHeight + 20.0f));
}

} // namespace BlindCard
