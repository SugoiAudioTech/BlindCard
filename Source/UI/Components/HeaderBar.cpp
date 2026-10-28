/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    HeaderBar.cpp
    Created: 2026-01-19
    Author:  BlindCard

    Implementation of the HeaderBar component.

  ==============================================================================
*/

#include "HeaderBar.h"
#include "BinaryData.h"
#include "../Theme/FontManager.h"

namespace BlindCard
{

namespace
{
    float getTextWidth(const juce::Font& font, const juce::String& text)
    {
        juce::GlyphArrangement glyphs;
        glyphs.addLineOfText(font, text, 0.0f, 0.0f);
        return glyphs.getBoundingBox(0, glyphs.getNumGlyphs(), true).getWidth();
    }
}

//==============================================================================
// Layout constants
namespace Layout
{
    // CURVA house-brand lockup, scaled in JUCE logical pixels.
    constexpr int logoWidth = 44;  // 329:298 source aspect at 40px high
    constexpr int logoHeight = 40;
    constexpr int logoLeftMargin = 16;
    constexpr int brandLeftMargin = 4;
    constexpr float sugoiFontSize = 19.0f;
    constexpr float productFontSize = 24.5f;
    constexpr float trailingSpadeSize = 21.0f;

    // Connection status capsule
    constexpr int capsuleHeight = 36;
    constexpr float capsuleCornerRadius = 18.0f;

    // Right controls
    constexpr int controlRightMargin = 16;
    constexpr int controlSpacing = 8;

    // Theme toggle (now just an icon, same size as other icon buttons)
    constexpr int toggleWidth = 32;
    constexpr int toggleHeight = 32;

    // Icon buttons
    constexpr int iconButtonSize = 32;

    // Reset chip button
    constexpr float resetChipCornerRadius = 13.0f;
}

//==============================================================================
HeaderBar::HeaderBar()
{
    // Subscribe to theme changes
    ThemeManager::getInstance().addChangeListener(this);

    // Enable mouse tracking for hover effects
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

HeaderBar::~HeaderBar()
{
    ThemeManager::getInstance().removeChangeListener(this);
    stopTimer();
}

//==============================================================================
void HeaderBar::setUpdateAvailable(bool available)
{
    if (showUpdateDot != available)
    {
        showUpdateDot = available;
        repaint(settingsButtonBounds);
    }
}

void HeaderBar::setStandaloneMode(bool enabled)
{
    if (standaloneMode != enabled)
    {
        standaloneMode = enabled;
        resized();
        repaint();
    }
}

void HeaderBar::setCurrentTrackInfo(float rmsDb, const juce::String& trackName)
{
    // In standalone mode, don't update track info (TransportBar handles this)
    if (standaloneMode)
        return;

    currentRMSdB = rmsDb;
    currentTrackName = trackName;

    // Start timer for continuous repaint (level meter needs smooth updates)
    if (!isTimerRunning())
        startTimer(kTimerIntervalMs);

    repaint(trackInfoBounds);
}

//==============================================================================
void HeaderBar::paint(juce::Graphics& g)
{
    auto& theme = ThemeManager::getInstance();
    bool isDark = theme.isDark();
    auto bounds = getLocalBounds().toFloat();

    // Relate the neutral masthead to the canvas while keeping controls distinct.
    juce::Colour bgColor = isDark ? Theme::Dark::HeaderSurface : juce::Colour(0xFFF5F0E8);
    g.setColour(bgColor);
    g.fillRect(bounds);

    // Bottom border separates the header without a bright rule.
    juce::Colour borderColor = isDark ? juce::Colour(0xFF626762) : juce::Colour(0xFFD4C9B8);
    g.setColour(borderColor);
    g.fillRect(bounds.removeFromBottom(1.0f));

    // Draw components
    float logoX = static_cast<float>(Layout::logoLeftMargin);
    float logoY = (bounds.getHeight() - Layout::logoHeight) / 2.0f;
    drawLogo(g, { logoX, logoY, static_cast<float>(Layout::logoWidth), static_cast<float>(Layout::logoHeight) });

    // Keep the brand lockup clear of the centre status/standalone transport
    // at the editor's 900px minimum width.
    float brandX = logoX + Layout::logoWidth + Layout::brandLeftMargin;
    float brandWidth = juce::jmax(0.0f, static_cast<float>(trackInfoBounds.getX()) - brandX - 12.0f);
    drawBrandText(g, { brandX, 0.0f, brandWidth, bounds.getHeight() });

    // Track info (centered) - hidden in standalone mode (TransportBar shows playback info)
    if (!standaloneMode)
    {
        drawTrackInfo(g, trackInfoBounds.toFloat());
    }

    // Right controls
    drawThemeToggle(g, themeToggleBounds.toFloat(), currentHover == HoverState::ThemeToggle);
    drawInfoButton(g, infoButtonBounds.toFloat(), currentHover == HoverState::InfoButton);
    drawSettingsButton(g, settingsButtonBounds.toFloat(), currentHover == HoverState::SettingsButton);
}

//==============================================================================
void HeaderBar::resized()
{
    auto bounds = getLocalBounds();
    auto centerY = bounds.getCentreY();

    // Calculate right-side button positions (from right to left)
    // 順序：說明 | 深淺切換 | 設定
    int rightEdge = bounds.getWidth() - Layout::controlRightMargin;

    // Settings button (最右邊)
    settingsButtonBounds = {
        rightEdge - Layout::iconButtonSize,
        centerY - Layout::iconButtonSize / 2,
        Layout::iconButtonSize,
        Layout::iconButtonSize
    };
    rightEdge = settingsButtonBounds.getX() - Layout::controlSpacing;

    // Theme toggle (中間)
    themeToggleBounds = {
        rightEdge - Layout::toggleWidth,
        centerY - Layout::toggleHeight / 2,
        Layout::toggleWidth,
        Layout::toggleHeight
    };
    rightEdge = themeToggleBounds.getX() - Layout::controlSpacing;

    // Info button (最左邊)
    infoButtonBounds = {
        rightEdge - Layout::iconButtonSize,
        centerY - Layout::iconButtonSize / 2,
        Layout::iconButtonSize,
        Layout::iconButtonSize
    };

    // Track info (centered in the header, wider to fit NOW PLAYING + RMS + track name)
    int trackInfoWidth = 360;  // Wider to fit all sections
    trackInfoBounds = {
        bounds.getCentreX() - trackInfoWidth / 2,
        centerY - Layout::capsuleHeight / 2,
        trackInfoWidth,
        Layout::capsuleHeight
    };
}

//==============================================================================
void HeaderBar::mouseMove(const juce::MouseEvent& event)
{
    auto newHover = getHoverStateAt(event.getPosition());
    if (newHover != currentHover)
    {
        currentHover = newHover;
        repaint();
    }
}

void HeaderBar::mouseExit(const juce::MouseEvent&)
{
    if (currentHover != HoverState::None)
    {
        currentHover = HoverState::None;
        repaint();
    }
}

void HeaderBar::mouseDown(const juce::MouseEvent& event)
{
    auto clickState = getHoverStateAt(event.getPosition());

    switch (clickState)
    {
        case HoverState::None:
        case HoverState::ResetButton:
            break;

        case HoverState::ThemeToggle:
            if (onThemeToggled)
                onThemeToggled();
            break;

        case HoverState::InfoButton:
            if (onInfoClicked)
                onInfoClicked();
            break;

        case HoverState::SettingsButton:
            if (onSettingsClicked)
                onSettingsClicked();
            break;
    }
}

//==============================================================================
void HeaderBar::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &ThemeManager::getInstance())
    {
        repaint();
    }
}

void HeaderBar::timerCallback()
{
    // Update pulse animation
    pulsePhase += static_cast<float>(kTimerIntervalMs) / kPulsePeriodMs;
    if (pulsePhase >= 1.0f)
        pulsePhase -= 1.0f;

    // Only repaint the track info area for efficiency
    repaint(trackInfoBounds);
}

//==============================================================================
void HeaderBar::drawLogo(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    static const juce::Image logoImage = juce::ImageCache::getFromMemory(
        BinaryData::sugoilogoheader_png, BinaryData::sugoilogoheader_pngSize);
    if (logoImage.isValid())
        g.drawImage(logoImage, bounds, juce::RectanglePlacement::centred);
}

void HeaderBar::drawBrandText(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    if (bounds.isEmpty())
        return;

    bool isDark = ThemeManager::getInstance().isDark();
    auto& fonts = FontManager::getInstance();
    auto sugoiFont = fonts.getMuseoModernoBold(Layout::sugoiFontSize)
                           .withExtraKerningFactor(0.01f);
    auto productFont = fonts.getMuseoModernoBold(Layout::productFontSize)
                             .withExtraKerningFactor(-0.01f);
    float sugoiWidth = getTextWidth(sugoiFont, "Sugoi") + 2.0f;
    float dividerX = bounds.getX() + sugoiWidth + 7.0f;
    float productX = dividerX + 9.0f;

    g.setColour(isDark ? Theme::Dark::TextPrimary : juce::Colour(0xFF2C2925));
    g.setFont(sugoiFont);
    g.drawText("Sugoi", bounds.withWidth(sugoiWidth), juce::Justification::centredLeft);

    g.setColour(isDark ? juce::Colour(0xFFB9C0B9) : juce::Colour(0xFFB5A798));
    g.fillRect(dividerX, bounds.getCentreY() - 12.5f, 1.0f, 25.0f);

    float blindWidth = getTextWidth(productFont, "Blind") + 1.0f;
    float cardWidth = getTextWidth(productFont, "Card") + 1.0f;
    float remaining = juce::jmax(0.0f, bounds.getRight() - productX);
    g.setFont(productFont);
    g.setColour(isDark ? Theme::Dark::TextPrimary : juce::Colour(0xFF25211F));
    g.drawText("Blind", juce::Rectangle<float>(productX, bounds.getY(),
                                                juce::jmin(blindWidth, remaining), bounds.getHeight()),
               juce::Justification::centredLeft);
    // The dark masthead needs a slightly lighter red for the large wordmark.
    g.setColour(isDark ? juce::Colour(0xFFF17B80) : juce::Colour(0xFFC9363C));
    g.drawText("Card", juce::Rectangle<float>(productX + blindWidth, bounds.getY(),
                                               juce::jmin(cardWidth, juce::jmax(0.0f, remaining - blindWidth)), bounds.getHeight()),
               juce::Justification::centredLeft);

    // The small boxed spade is the original BlindCard mark; the unboxed Sugoi
    // animal at the far left remains the house-brand logo.
    float tileX = productX + blindWidth + cardWidth + 5.0f;
    if (tileX + Layout::trailingSpadeSize <= bounds.getRight())
        drawTrailingSpade(g, { tileX, bounds.getCentreY() - Layout::trailingSpadeSize * 0.5f,
                               Layout::trailingSpadeSize, Layout::trailingSpadeSize });
}

void HeaderBar::drawTrailingSpade(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    const bool dark = ThemeManager::getInstance().isDark();
    juce::ColourGradient red(
        dark ? juce::Colour(0xFFFF3B4E) : juce::Colour(0xFFC41E3A), bounds.getX(), bounds.getY(),
        dark ? juce::Colour(0xFFB82D3A) : juce::Colour(0xFF8B1528), bounds.getRight(), bounds.getBottom(),
        false);
    g.setGradientFill(red);
    g.fillRoundedRectangle(bounds, 4.0f);

    auto inner = bounds.reduced(bounds.getWidth() * 0.18f);
    const float x0 = inner.getX(), y0 = inner.getY();
    const float w = inner.getWidth(), h = inner.getHeight();
    auto sx = [x0, w](float v) { return x0 + v * w / 100.0f; };
    auto sy = [y0, h](float v) { return y0 + v * h / 100.0f; };

    juce::Path spade;
    spade.startNewSubPath(sx(50), sy(0));
    spade.cubicTo(sx(50), sy(0), sx(0), sy(40), sx(0), sy(60));
    spade.cubicTo(sx(0), sy(80), sx(25), sy(85), sx(50), sy(70));
    spade.cubicTo(sx(75), sy(85), sx(100), sy(80), sx(100), sy(60));
    spade.cubicTo(sx(100), sy(40), sx(50), sy(0), sx(50), sy(0));
    spade.closeSubPath();

    juce::Path stem;
    stem.startNewSubPath(sx(45), sy(65));
    stem.lineTo(sx(38), sy(100));
    stem.lineTo(sx(62), sy(100));
    stem.lineTo(sx(55), sy(65));
    stem.closeSubPath();

    g.setColour(juce::Colours::white);
    g.fillPath(spade);
    g.fillPath(stem);
}

void HeaderBar::drawTrackInfo(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& theme = ThemeManager::getInstance();
    auto& fonts = FontManager::getInstance();
    bool isDark = theme.isDark();

    // Low-key charcoal capsule in the dark masthead.
    juce::Colour bgColor = isDark ? juce::Colour(0xFF484B49) : juce::Colour(0xFFFDF6E3);
    g.setColour(bgColor);
    g.fillRoundedRectangle(bounds, Layout::capsuleCornerRadius);

    // Capsule border - Original: rgba(212, 175, 55, 0.3) dark / 0.5 light
    juce::Colour borderColor = isDark
        ? juce::Colour(0xFF727772)
        : juce::Colour(0xFFD4AF37).withAlpha(0.5f);
    g.setColour(borderColor);
    g.drawRoundedRectangle(bounds.reduced(0.5f), Layout::capsuleCornerRadius, 1.0f);

    // Content area with padding
    auto contentArea = bounds.reduced(12.0f, 0.0f);

    // Colors
    juce::Colour goldColor = isDark ? Theme::Dark::Accent : juce::Colour(0xFFD4AF37);
    juce::Colour textColor = isDark ? Theme::Dark::TextSecondary : juce::Colour(0xFF9C7E25);

    // Format: Now Playing | 軌道名稱 | dB

    // "Now Playing" section (left side)
    auto nowPlayingArea = contentArea.removeFromLeft(90.0f);
    g.setColour(goldColor);
    g.setFont(fonts.getMedium(14.0f));
    g.drawText("NOW PLAYING", nowPlayingArea, juce::Justification::centred);

    // Vertical separator line
    contentArea.removeFromLeft(8.0f);
    auto separatorArea = contentArea.removeFromLeft(1.0f);
    g.setColour(borderColor);
    g.fillRect(separatorArea.reduced(0.0f, 6.0f));
    contentArea.removeFromLeft(8.0f);

    // Track name (middle section)
    auto trackNameArea = contentArea.removeFromLeft(140.0f);
    g.setFont(fonts.getMedium(14.0f));
    g.setColour(textColor);

    if (currentTrackName.isEmpty())
    {
        g.drawText("NO TRACK", trackNameArea, juce::Justification::centred);
    }
    else
    {
        g.drawText(currentTrackName.toUpperCase(), trackNameArea, juce::Justification::centred);
    }

    // Vertical separator line
    contentArea.removeFromLeft(6.0f);
    auto separator2Area = contentArea.removeFromLeft(1.0f);
    g.setColour(borderColor);
    g.fillRect(separator2Area.reduced(0.0f, 6.0f));
    contentArea.removeFromLeft(8.0f);

    // RMS display (right side)
    juce::String rmsText;
    if (currentRMSdB <= -60.0f)
    {
        rmsText = "-- dB";
    }
    else
    {
        rmsText = juce::String(currentRMSdB, 1) + " dB";
    }

    g.setFont(fonts.getMedium(14.0f));
    g.setColour(goldColor);
    g.drawText(rmsText, contentArea, juce::Justification::centred);
}

void HeaderBar::drawThemeToggle(juce::Graphics& g, juce::Rectangle<float> bounds, bool isHovered)
{
    auto& theme = ThemeManager::getInstance();
    bool isDark = theme.isDark();

    // Draw hover background with shadow (AirCheck style)
    if (isHovered)
    {
        // Expand bounds for background
        auto bgBounds = bounds.expanded(4.0f);

        // Shadow (subtle drop shadow)
        g.setColour(juce::Colours::black.withAlpha(0.15f));
        g.fillRoundedRectangle(bgBounds.translated(0.0f, 2.0f), 10.0f);

        // Background
        juce::Colour bgColor = isDark ? Theme::Dark::SurfaceAlt : juce::Colour(0xFFEEEEEE);
        g.setColour(bgColor);
        g.fillRoundedRectangle(bgBounds, 10.0f);
    }

    // Golden/yellow color for sun and moon
    juce::Colour iconColor = isDark ? Theme::Dark::Accent : juce::Colour(0xFFFFD700);

    // Icon size and position
    float iconSize = 22.0f;
    auto iconBounds = bounds.withSizeKeepingCentre(iconSize, iconSize);

    if (isDark)
    {
        // Dark mode: show SUN icon (to switch to light)
        // Draw sun with rays
        float centerX = iconBounds.getCentreX();
        float centerY = iconBounds.getCentreY();
        float sunRadius = 6.0f;
        float rayLength = 4.0f;
        float rayOffset = sunRadius + 2.0f;

        // Sun body (filled circle)
        g.setColour(iconColor);
        g.fillEllipse(centerX - sunRadius, centerY - sunRadius, sunRadius * 2.0f, sunRadius * 2.0f);

        // Sun rays (8 rays)
        g.setColour(iconColor);
        for (int i = 0; i < 8; ++i)
        {
            float angle = static_cast<float>(i) * juce::MathConstants<float>::pi / 4.0f;
            float x1 = centerX + rayOffset * std::cos(angle);
            float y1 = centerY + rayOffset * std::sin(angle);
            float x2 = centerX + (rayOffset + rayLength) * std::cos(angle);
            float y2 = centerY + (rayOffset + rayLength) * std::sin(angle);
            g.drawLine(x1, y1, x2, y2, 2.0f);
        }
    }
    else
    {
        // Light mode: show MOON icon (to switch to dark)
        // Draw crescent moon
        float centerX = iconBounds.getCentreX();
        float centerY = iconBounds.getCentreY();
        float moonRadius = 8.0f;

        // Create crescent by subtracting a circle
        juce::Path moon;
        moon.addEllipse(centerX - moonRadius, centerY - moonRadius, moonRadius * 2.0f, moonRadius * 2.0f);

        // Subtract overlapping circle to create crescent
        juce::Path cutout;
        float cutoutOffset = 5.0f;
        cutout.addEllipse(centerX - moonRadius + cutoutOffset, centerY - moonRadius - 1.0f,
                          moonRadius * 2.0f, moonRadius * 2.0f);

        // Use path subtraction
        juce::Path crescent;
        crescent.addPath(moon);

        g.setColour(iconColor);
        g.fillPath(moon);

        // Draw cutout in background color to create crescent effect
        auto bgColor = theme.isDark() ? Theme::Dark::HeaderSurface : juce::Colour(0xFFF5F0E8);
        g.setColour(bgColor);
        g.fillPath(cutout);
    }
}

void HeaderBar::drawInfoButton(juce::Graphics& g, juce::Rectangle<float> bounds, bool isHovered)
{
    auto& theme = ThemeManager::getInstance();
    bool isDark = theme.isDark();

    // Draw hover background with shadow (AirCheck style)
    if (isHovered)
    {
        auto bgBounds = bounds.expanded(4.0f);

        // Shadow
        g.setColour(juce::Colours::black.withAlpha(0.15f));
        g.fillRoundedRectangle(bgBounds.translated(0.0f, 2.0f), 10.0f);

        // Background
        juce::Colour bgColor = isDark ? Theme::Dark::SurfaceAlt : juce::Colour(0xFFEEEEEE);
        g.setColour(bgColor);
        g.fillRoundedRectangle(bgBounds, 10.0f);
    }

    // Icon color - gray outline style
    juce::Colour iconColor = isDark ? juce::Colour(0xFFD2CBBF) : juce::Colour(0xFF606060);

    if (isHovered)
        iconColor = isDark ? Theme::Dark::TextPrimary : juce::Colour(0xFF404040);

    // Question mark icon (circle with '?')
    g.setColour(iconColor);

    // Draw circle outline
    float iconSize = 22.0f;
    auto iconBounds = bounds.withSizeKeepingCentre(iconSize, iconSize);
    g.drawEllipse(iconBounds, 2.0f);

    // Draw '?' character
    g.setFont(FontManager::getInstance().getBold(16.0f));
    g.drawText("?", iconBounds.toNearestInt(), juce::Justification::centred);
}

void HeaderBar::drawSettingsButton(juce::Graphics& g, juce::Rectangle<float> bounds, bool isHovered)
{
    auto& theme = ThemeManager::getInstance();
    bool isDark = theme.isDark();

    // Draw hover background with shadow (AirCheck style)
    if (isHovered)
    {
        auto bgBounds = bounds.expanded(4.0f);

        // Shadow
        g.setColour(juce::Colours::black.withAlpha(0.15f));
        g.fillRoundedRectangle(bgBounds.translated(0.0f, 2.0f), 10.0f);

        // Background
        juce::Colour bgColor = isDark ? Theme::Dark::SurfaceAlt : juce::Colour(0xFFEEEEEE);
        g.setColour(bgColor);
        g.fillRoundedRectangle(bgBounds, 10.0f);
    }

    // Icon color - gray outline style matching the design
    juce::Colour iconColor = isDark ? juce::Colour(0xFFD2CBBF) : juce::Colour(0xFF606060);

    if (isHovered)
        iconColor = isDark ? Theme::Dark::TextPrimary : juce::Colour(0xFF404040);

    g.setColour(iconColor);

    float centerX = bounds.getCentreX();
    float centerY = bounds.getCentreY();
    float outerRadius = 10.0f;
    float innerRadius = 4.5f;
    int numTeeth = 8;

    // Draw gear shape as outline
    juce::Path gear;
    for (int i = 0; i < numTeeth * 2; ++i)
    {
        float angle = static_cast<float>(i) * juce::MathConstants<float>::pi / static_cast<float>(numTeeth);
        float radius = (i % 2 == 0) ? outerRadius : outerRadius - 3.0f;

        float x = centerX + radius * std::cos(angle);
        float y = centerY + radius * std::sin(angle);

        if (i == 0)
            gear.startNewSubPath(x, y);
        else
            gear.lineTo(x, y);
    }
    gear.closeSubPath();

    // Draw gear outline instead of fill
    g.strokePath(gear, juce::PathStrokeType(2.0f));

    // Draw center circle outline
    g.drawEllipse(centerX - innerRadius, centerY - innerRadius,
                  innerRadius * 2.0f, innerRadius * 2.0f, 2.0f);

    // Update notification dot (top-right of gear icon)
    if (showUpdateDot)
    {
        float dotSize = 8.0f;
        float dotX = bounds.getRight() - dotSize / 2.0f - 1.0f;
        float dotY = bounds.getY() - dotSize / 2.0f + 3.0f;

        g.setColour(juce::Colours::white);
        g.fillEllipse(dotX - 1.0f, dotY - 1.0f, dotSize + 2.0f, dotSize + 2.0f);
        g.setColour(isDark ? juce::Colour(0xFF22C55E) : juce::Colour(0xFF16A34A));
        g.fillEllipse(dotX, dotY, dotSize, dotSize);
    }
}

void HeaderBar::drawResetButton(juce::Graphics& g, juce::Rectangle<float> bounds, bool isHovered)
{
    auto& theme = ThemeManager::getInstance();
    bool isDark = theme.isDark();

    // Red chip background (like poker chip)
    juce::Colour baseColor = isDark ? juce::Colour(0xFFB82D3A) : juce::Colour(0xFFC41E3A);
    juce::Colour highlightColor = isDark ? juce::Colour(0xFFFF3B4E) : juce::Colour(0xFFE53E4E);

    if (isHovered)
        baseColor = highlightColor;

    // Draw chip body with gradient
    juce::ColourGradient gradient(
        highlightColor, bounds.getX(), bounds.getY(),
        baseColor, bounds.getRight(), bounds.getBottom(),
        false);

    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, Layout::resetChipCornerRadius);

    // Chip edge highlight
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawRoundedRectangle(bounds.reduced(1.0f), Layout::resetChipCornerRadius - 1.0f, 1.0f);

    // Inner shadow (top)
    g.setColour(juce::Colours::black.withAlpha(0.15f));
    auto innerBounds = bounds.reduced(2.0f);
    g.drawLine(innerBounds.getX() + 4.0f, innerBounds.getY(),
               innerBounds.getRight() - 4.0f, innerBounds.getY(), 1.0f);

    // "RESET" label uses Noto Sans TC Medium
    g.setColour(juce::Colours::white);
    g.setFont(FontManager::getInstance().getMedium(12.0f));
    g.drawText("RESET", bounds.toNearestInt(), juce::Justification::centred);
}

//==============================================================================
HeaderBar::HoverState HeaderBar::getHoverStateAt(juce::Point<int> position) const
{
    if (themeToggleBounds.contains(position))
        return HoverState::ThemeToggle;

    if (infoButtonBounds.contains(position))
        return HoverState::InfoButton;

    if (settingsButtonBounds.contains(position))
        return HoverState::SettingsButton;

    return HoverState::None;
}

} // namespace BlindCard
