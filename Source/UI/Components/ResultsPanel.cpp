/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    ResultsPanel.cpp
    Created: 2026-01-19
    Author:  BlindCard

  ==============================================================================
*/

#include "ResultsPanel.h"
#include "../Theme/FontManager.h"
#include "../Localization/LocalizationManager.h"
#include <algorithm>

namespace BlindCard
{

namespace
{
juce::Colour resultSuccessInk(bool dark) { return juce::Colour(dark ? 0xFFBBDDC5 : 0xFF21633F); }
juce::Colour resultErrorInk(bool dark)   { return juce::Colour(dark ? 0xFFFAC6C7 : 0xFF9A3238); }
juce::Colour resultScoreInk(bool dark)   { return juce::Colour(dark ? 0xFFE7CF9F : 0xFF765522); }
}

//==============================================================================
ResultsPanel::ResultsPanel()
{
    ThemeManager::getInstance().addChangeListener(this);

    contentComponent = std::make_unique<ContentComponent>(*this);
    viewport = std::make_unique<juce::Viewport>();
    viewport->setViewedComponent(contentComponent.get(), false);
    viewport->setScrollBarsShown(true, false);
    viewport->setScrollBarThickness(8);
    addAndMakeVisible(*viewport);

    // Create submit button for Guess mode
    submitButton = std::make_unique<juce::TextButton>("SUBMIT GUESSES");
    submitButton->onClick = [this]()
    {
        if (onSubmitGuesses)
            onSubmitGuesses();
    };
    contentComponent->addChildComponent(*submitButton);  // Hidden by default
    changeListenerCallback(nullptr);
}

ResultsPanel::~ResultsPanel()
{
    ThemeManager::getInstance().removeChangeListener(this);
    viewport->setViewedComponent(nullptr, false);
}

//==============================================================================
void ResultsPanel::setMode(blindcard::RatingMode mode)
{
    if (currentMode == mode) return;
    currentMode = mode;
    submitButton->setVisible(mode == blindcard::RatingMode::Guess && !guessResultsVisible);
    updateContentLayout(true);
    repaint();
}

//==============================================================================
void ResultsPanel::setStarsResults(const std::vector<StarsResult>& results)
{
    starsResults = results;

    // Sort order depends on phase:
    // - BlindTesting: fixed order by card position (no jumping around)
    // - Revealed: sort by average rating (highest first) to show ranking
    std::sort(starsResults.begin(), starsResults.end(),
              [this](const StarsResult& a, const StarsResult& b)
              {
                  if (currentPhase == blindcard::GamePhase::Revealed)
                      return a.averageRating > b.averageRating;
                  else
                      return a.cardPosition < b.cardPosition;  // Fixed order during blind testing
              });

    updateContentLayout();
}

void ResultsPanel::setGuessResults(const std::vector<GuessResult>& results)
{
    guessResults = results;
    updateContentLayout();
}

void ResultsPanel::setSubmitEnabled(bool enabled)
{
    submitEnabled = enabled;
    submitButton->setEnabled(enabled);
}

void ResultsPanel::setGuessResultsVisible(bool showResults)
{
    if (guessResultsVisible == showResults) return;
    guessResultsVisible = showResults;
    submitButton->setVisible(!showResults && currentMode == blindcard::RatingMode::Guess);
    updateContentLayout();
}

void ResultsPanel::setQAResults(const std::vector<QAResult>& results)
{
    qaResults = results;
    updateContentLayout();
}

void ResultsPanel::clearResults()
{
    starsResults.clear();
    guessResults.clear();
    qaResults.clear();
    guessResultsVisible = false;
    showFinalResults = false;
    roundCorrectGuesses = roundGuessAttempts = 0;
    totalCorrectGuesses = totalGuessAttempts = 0;
    countdownSeconds = 0;
    submitButton->setVisible(currentMode == blindcard::RatingMode::Guess);
    updateContentLayout(true);
    repaint();
}

void ResultsPanel::setShowFinalResults(bool showFinal)
{
    if (showFinalResults != showFinal)
    {
        showFinalResults = showFinal;
        updateContentLayout();
    }
}

void ResultsPanel::setRoundInfo(int current, int total)
{
    const bool changed = currentRoundNumber != current || totalRounds != total;
    currentRoundNumber = current;
    totalRounds = total;
    if (changed) updateContentLayout(true);
}

void ResultsPanel::setGuessScore(int roundCorrect, int roundTotal,
                                 int cumulativeCorrect, int cumulativeTotal)
{
    roundCorrectGuesses = roundCorrect;
    roundGuessAttempts = roundTotal;
    totalCorrectGuesses = cumulativeCorrect;
    totalGuessAttempts = cumulativeTotal;
    contentComponent->repaint();
}

void ResultsPanel::setResultsSession(std::uint64_t sessionId)
{
    if (resultSessionId == sessionId) return;
    resultSessionId = sessionId;
    countdownSeconds = 0;
    updateContentLayout(true);
}

void ResultsPanel::setCountdown(int seconds)
{
    if (countdownSeconds != seconds)
    {
        countdownSeconds = seconds;
        contentComponent->repaint();
    }
}

void ResultsPanel::setPhase(blindcard::GamePhase phase)
{
    if (currentPhase != phase)
    {
        currentPhase = phase;
        contentComponent->repaint();
        repaint();
    }
}

//==============================================================================
void ResultsPanel::changeListenerCallback(juce::ChangeBroadcaster*)
{
    // Update button colors for theme
    auto& tm = ThemeManager::getInstance();
    submitButton->setColour(juce::TextButton::buttonColourId, tm.getColour(ColourId::Primary));
    submitButton->setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    viewport->getVerticalScrollBar().setColour(juce::ScrollBar::thumbColourId,
        tm.isDark() ? juce::Colour(0xFF858A85) : juce::Colour(0xFFB8AEA0));
    contentComponent->repaint();
    repaint();
}

//==============================================================================
void ResultsPanel::paint(juce::Graphics& g)
{
    auto& tm = ThemeManager::getInstance();
    auto bounds = getLocalBounds().toFloat();
    bool isDark = tm.isDark();

    // Share the raised dark surface with the neighbouring control panel.
    juce::Colour bgColor = isDark ? tm.getColour(ColourId::Surface) : juce::Colour(0xFFF5F0E8);
    g.setColour(bgColor);
    g.fillRoundedRectangle(bounds, 12.0f);  // Original: rounded-xl (~12px)

    // Both lower panels share the same quiet charcoal edge.
    juce::Colour borderColor = isDark ? juce::Colour(0xFF737873) : juce::Colour(0xFFD4C5A9);
    g.setColour(borderColor);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 12.0f, 1.0f);

    // Header stays fixed while rows, score, and Submit live inside the viewport.
    auto contentBounds = bounds.reduced(20.0f);
    auto headerBounds = contentBounds.removeFromTop(28.0f);
    drawSectionHeader(g, headerBounds);
}

void ResultsPanel::drawContent(juce::Graphics& g)
{
    auto contentBounds = contentComponent->getLocalBounds().toFloat();
    switch (currentMode)
    {
        case blindcard::RatingMode::Stars:
            if (starsResults.empty())
                drawEmptyState(g, contentBounds);
            else
                drawStarsResults(g, contentBounds);
            break;

        case blindcard::RatingMode::Guess:
            if (guessResults.empty())
            {
                if (!guessResultsVisible)
                    contentBounds.removeFromBottom(48.0f);
                drawEmptyState(g, contentBounds);
            }
            else if (guessResultsVisible)
                drawGuessResults(g, contentBounds);
            else
            {
                contentBounds.removeFromBottom(48.0f); // Submit has its own child hit area.
                drawGuessPending(g, contentBounds);
            }
            break;

        case blindcard::RatingMode::QA:
            if (qaResults.empty())
                drawEmptyState(g, contentBounds);
            else
                drawQAResults(g, contentBounds);
            break;
    }
}

void ResultsPanel::drawSectionHeader(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();
    auto& fonts = FontManager::getInstance();
    bool isDark = tm.isDark();

    // Gold color for Trophy icon - Original: #FFD700 (dark) / #B8860B (light)
    juce::Colour goldColor = isDark ? Theme::Dark::Accent : juce::Colour(0xFFB8860B);

    // Trophy icon (🏆) - Original: w-5 h-5
    g.setColour(goldColor);
    g.setFont(fonts.getRegular(24.0f));
    auto iconBounds = bounds.removeFromLeft(28.0f);
    g.drawText(juce::String::fromUTF8("🏆"), iconBounds, juce::Justification::centred);

    bounds.removeFromLeft(8.0f);  // gap-2

    // Readable Noto Sans TC Bold title
    juce::Colour titleColor = isDark ? Theme::Dark::TextPrimary : juce::Colour(0xFF1A1A1A);
    g.setColour(titleColor);
    g.setFont(fonts.getBold(19.0f));

    // Show "FINAL RESULTS" when revealed or when explicitly showing final results
    bool isFinal = showFinalResults || (currentPhase == blindcard::GamePhase::Revealed);
    g.drawText(isFinal ? LOCALIZE(ResultsFinalResults) : LOCALIZE(ResultsCurrentRound),
               bounds, juce::Justification::centredLeft);
}

void ResultsPanel::drawEmptyState(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();
    auto& fonts = FontManager::getInstance();
    bool isDark = tm.isDark();

    // Center the content vertically
    auto centerY = bounds.getCentreY();

    // Circle background for star icon - Original: w-12 h-12 (48px), bg-[#252525] (dark) / bg-[#E8E0D0] (light)
    float circleSize = 48.0f;
    auto circleBounds = juce::Rectangle<float>(
        bounds.getCentreX() - circleSize / 2.0f,
        centerY - circleSize / 2.0f - 16.0f,  // Offset up to leave room for text
        circleSize, circleSize
    );

    juce::Colour circleBgColor = isDark ? juce::Colour(0xFF454846) : juce::Colour(0xFFE8E0D0);
    g.setColour(circleBgColor);
    g.fillEllipse(circleBounds);

    // Star icon inside circle - Original: w-6 h-6 (24px), #333 (dark) / #C4B5A0 (light)
    juce::Colour starColor = isDark ? juce::Colour(0xFFAAB2AB) : juce::Colour(0xFFC4B5A0);
    g.setColour(starColor);
    g.setFont(fonts.getRegular(28.0f));
    g.drawText(juce::String::fromUTF8("★"), circleBounds, juce::Justification::centred);

    // Message text below - AirCheck style: muted text
    juce::Colour textColor = isDark ? Theme::Dark::TextMuted : juce::Colour(0xFF888888);
    g.setColour(textColor);
    g.setFont(fonts.getRegular(16.0f));

    juce::String message;
    switch (currentMode)
    {
        case blindcard::RatingMode::Stars:
            message = "Rate cards to see results";
            break;
        case blindcard::RatingMode::Guess:
            message = "Make guesses to see results";
            break;
        case blindcard::RatingMode::QA:
            message = LOCALIZE(ResultsAnswerQuestions);
            break;
    }

    auto textBounds = juce::Rectangle<float>(
        bounds.getX(),
        circleBounds.getBottom() + 12.0f,  // mb-3 = 12px gap
        bounds.getWidth(),
        20.0f
    );
    g.drawText(message, textBounds, juce::Justification::centred);
}

//==============================================================================
// Stars mode drawing

void ResultsPanel::drawStarsResults(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();
    bool isDark = tm.isDark();

    // Readable rows may extend beyond the 220px panel; the viewport scrolls them.
    int numResults = static_cast<int>(starsResults.size());
    if (numResults == 0) return;

    float availableHeight = bounds.getHeight();
    float minRowHeight = 24.0f;
    float maxRowHeight = 32.0f;
    float minGap = 4.0f;

    // Calculate row height to fit all results
    float totalSpacePerRow = (availableHeight - minGap) / numResults;
    float rowHeight = juce::jlimit(minRowHeight, maxRowHeight, totalSpacePerRow - minGap);
    float rowGap = minGap;

    for (size_t i = 0; i < starsResults.size(); ++i)
    {
        auto rowBounds = bounds.removeFromTop(rowHeight);

        // Row background - Original: bg-[#252525] (dark) / bg-[#E8E0D0] (light)
        juce::Colour rowBgColor = isDark ? juce::Colour(0xFF454846) : juce::Colour(0xFFE8E0D0);
        g.setColour(rowBgColor);
        g.fillRoundedRectangle(rowBounds, 6.0f);  // rounded-lg

        // Pass rank (1-based) - results are sorted by rating (highest first)
        int rank = static_cast<int>(i) + 1;
        drawStarsRow(g, rowBounds.reduced(6.0f, 2.0f), starsResults[i], rank);

        if (i < starsResults.size() - 1)
            bounds.removeFromTop(rowGap);
    }
}

void ResultsPanel::drawStarsRow(juce::Graphics& g, juce::Rectangle<float> bounds,
                                 const StarsResult& result, int rank)
{
    auto& tm = ThemeManager::getInstance();
    auto& fonts = FontManager::getInstance();
    bool isDark = tm.isDark();

    // Star colors - Original: #FFD700 filled (dark) / #B8860B filled (light)
    // Empty: #333 (dark) / #C4B5A0 (light)
    juce::Colour filledStarColor = isDark ? Theme::Dark::Accent : juce::Colour(0xFFB8860B);
    juce::Colour emptyStarColor = isDark ? juce::Colour(0xFFAAB2AB) : juce::Colour(0xFFC4B5A0);

    // Track name on left - Original: text-sm, white (dark) / #1A1A1A (light)
    juce::Colour textColor = isDark ? Theme::Dark::TextPrimary : juce::Colour(0xFF1A1A1A);

    // Always reserve space for rank indicator to maintain alignment
    auto rankArea = bounds.removeFromLeft(24.0f);

    // Show rank indicator only in Revealed phase
    if (currentPhase == blindcard::GamePhase::Revealed)
    {
        g.setFont(fonts.getMedium(14.0f));
        if (rank == 1)
        {
            // Winner gets gold medal
            g.setColour(filledStarColor);
            g.drawText(juce::String::fromUTF8("🥇"), rankArea, juce::Justification::centred);
        }
        else if (rank == 2)
        {
            // Second place gets silver medal
            g.setColour(textColor.withAlpha(0.8f));
            g.drawText(juce::String::fromUTF8("🥈"), rankArea, juce::Justification::centred);
        }
        else if (rank == 3)
        {
            // Third place gets bronze medal
            g.setColour(textColor.withAlpha(0.7f));
            g.drawText(juce::String::fromUTF8("🥉"), rankArea, juce::Justification::centred);
        }
        else
        {
            // Other ranks show number
            g.setColour(textColor.withAlpha(0.5f));
            g.drawText(juce::String(rank), rankArea, juce::Justification::centred);
        }
    }

    g.setColour(textColor);
    g.setFont(fonts.getMedium(15.0f));  // Match ModeSelector font style

    // During BlindTesting, hide track name - show "Card X" instead
    juce::String displayName;
    if (currentPhase == blindcard::GamePhase::BlindTesting)
    {
        displayName = "Card " + juce::String(result.cardPosition + 1);
    }
    else
    {
        // Truncate track name if needed (show first part before " - ")
        displayName = juce::String(result.trackName);
        if (displayName.contains(" - "))
            displayName = displayName.upToFirstOccurrenceOf(" - ", false, false);
    }

    auto nameArea = bounds.removeFromLeft(bounds.getWidth() * 0.55f);
    g.drawText(displayName, nameArea, juce::Justification::centredLeft);

    // Stars area
    auto starsArea = bounds;
    float starSize = 12.0f;
    float starGap = 2.0f;
    float totalStarsWidth = 5 * starSize + 4 * starGap;

    // During Revealed phase: show average rating with numeric value
    // During BlindTesting: show current round's rating (integer stars only)
    bool showAverage = (currentPhase == blindcard::GamePhase::Revealed);

    int starsToFill = 0;
    if (showAverage)
    {
        // Round average to nearest integer for star display
        starsToFill = static_cast<int>(result.averageRating + 0.5f);
    }
    else
    {
        starsToFill = result.currentRating;
    }

    // Draw 5 stars
    float startX = starsArea.getX();
    g.setFont(fonts.getRegular(starSize));
    for (int i = 0; i < 5; ++i)
    {
        bool filled = i < starsToFill;
        g.setColour(filled ? filledStarColor : emptyStarColor);

        auto starBounds = juce::Rectangle<float>(
            startX + i * (starSize + starGap),
            starsArea.getCentreY() - starSize / 2.0f,
            starSize, starSize
        );
        g.drawText(juce::String::fromUTF8("★"), starBounds, juce::Justification::centred);
    }

    // Only show numeric average during Revealed phase
    if (showAverage)
    {
        float numericX = startX + totalStarsWidth + 6.0f;
        auto numericBounds = juce::Rectangle<float>(
            numericX,
            starsArea.getY(),
            starsArea.getRight() - numericX,
            starsArea.getHeight()
        );

        g.setColour(filledStarColor);
        g.setFont(fonts.getBold(16.0f));  // Match ModeSelector font style

        // Format average rating with one decimal place
        juce::String avgText = juce::String(result.averageRating, 1);
        g.drawText(avgText, numericBounds, juce::Justification::centredLeft);
    }
}

//==============================================================================
// Guess mode drawing

void ResultsPanel::drawGuessResults(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();
    bool isDark = tm.isDark();

    // Calculate dynamic row height
    int numResults = static_cast<int>(guessResults.size());
    if (numResults == 0) return;

    // Reserve space for score section at bottom
    float scoreHeight = showFinalResults ? 60.0f : 40.0f;
    auto scoreBounds = bounds.removeFromBottom(scoreHeight);

    float availableHeight = bounds.getHeight();
    float minRowHeight = 22.0f;
    float maxRowHeight = 32.0f;
    float rowGap = 3.0f;

    // Calculate row height: for N rows, we need N * rowHeight + (N-1) * rowGap
    // So: rowHeight = (availableHeight - (N-1) * rowGap) / N
    int numGaps = numResults > 1 ? numResults - 1 : 0;
    float totalGapSpace = numGaps * rowGap;
    float spaceForRows = availableHeight - totalGapSpace;
    float rowHeight = juce::jlimit(minRowHeight, maxRowHeight, spaceForRows / numResults);

    // Results list - draw all rows
    for (size_t i = 0; i < guessResults.size(); ++i)
    {
        auto rowBounds = bounds.removeFromTop(rowHeight);

        // Row background
        juce::Colour rowBgColor = isDark ? juce::Colour(0xFF454846) : juce::Colour(0xFFE8E0D0);
        g.setColour(rowBgColor);
        g.fillRoundedRectangle(rowBounds, 6.0f);

        drawGuessRow(g, rowBounds.reduced(6.0f, 2.0f), guessResults[i]);

        if (i < guessResults.size() - 1)
            bounds.removeFromTop(rowGap);
    }

    // Score summary
    drawGuessScore(g, scoreBounds);
}

void ResultsPanel::drawGuessPending(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();
    auto& fonts = FontManager::getInstance();
    bool isDark = tm.isDark();

    // Calculate dynamic row height based on number of results
    int numResults = static_cast<int>(guessResults.size());
    if (numResults == 0) return;

    float availableHeight = bounds.getHeight();
    float minRowHeight = 22.0f;
    float maxRowHeight = 32.0f;
    float rowGap = 3.0f;

    // Calculate row height: for N rows, we need N * rowHeight + (N-1) * rowGap
    int numGaps = numResults > 1 ? numResults - 1 : 0;
    float totalGapSpace = numGaps * rowGap;
    float spaceForRows = availableHeight - totalGapSpace;
    float rowHeight = juce::jlimit(minRowHeight, maxRowHeight, spaceForRows / numResults);

    // Fixed column widths for alignment
    float cardColWidth = 80.0f;
    float gapWidth = 16.0f;

    // Draw all rows
    for (size_t i = 0; i < guessResults.size(); ++i)
    {
        auto rowBounds = bounds.removeFromTop(rowHeight);
        auto& result = guessResults[i];

        // Row background for better readability
        juce::Colour rowBgColor = isDark ? juce::Colour(0xFF454846) : juce::Colour(0xFFE8E0D0);
        g.setColour(rowBgColor);
        g.fillRoundedRectangle(rowBounds, 6.0f);

        auto contentBounds = rowBounds.reduced(8.0f, 2.0f);

        // Card number column (fixed width, right-aligned)
        auto cardCol = contentBounds.removeFromLeft(cardColWidth);
        g.setColour(tm.getColour(ColourId::TextSecondary));
        g.setFont(fonts.getMedium(13.0f));
        g.drawText("Card " + juce::String(result.cardPosition + 1) + ":",
                   cardCol, juce::Justification::centredRight);

        contentBounds.removeFromLeft(gapWidth);

        // Guessed track (or "Not guessed")
        if (!result.guessedTrack.empty())
        {
            g.setColour(tm.getColour(ColourId::TextPrimary));
            g.setFont(fonts.getMedium(13.0f));
            g.drawText(juce::String(result.guessedTrack), contentBounds, juce::Justification::centredLeft);
        }
        else
        {
            g.setColour(tm.getColour(ColourId::TextMuted));
            g.setFont(fonts.getMedium(13.0f));
            g.drawText("Not guessed", contentBounds, juce::Justification::centredLeft);
        }

        if (i < guessResults.size() - 1)
            bounds.removeFromTop(rowGap);
    }
}

void ResultsPanel::drawGuessRow(juce::Graphics& g, juce::Rectangle<float> bounds,
                                 const GuessResult& result)
{
    auto& tm = ThemeManager::getInstance();
    auto& fonts = FontManager::getInstance();

    // Correct/wrong icon
    auto iconArea = bounds.removeFromLeft(22.0f);
    g.setFont(fonts.getMedium(14.0f));
    if (result.isCorrect)
    {
        g.setColour(resultSuccessInk(tm.isDark()));
        g.drawText(juce::String::fromUTF8("✓"), iconArea, juce::Justification::centred);
    }
    else
    {
        g.setColour(resultErrorInk(tm.isDark()));
        g.drawText(juce::String::fromUTF8("✗"), iconArea, juce::Justification::centred);
    }

    bounds.removeFromLeft(4.0f);

    // Card number column (fixed width, right-aligned for alignment)
    auto cardCol = bounds.removeFromLeft(70.0f);
    g.setColour(tm.getColour(ColourId::TextSecondary));
    g.setFont(fonts.getMedium(12.0f));
    g.drawText("Card " + juce::String(result.cardPosition + 1) + ":",
               cardCol, juce::Justification::centredRight);

    bounds.removeFromLeft(8.0f);

    // Result text
    g.setFont(fonts.getMedium(12.0f));
    if (result.isCorrect)
    {
        g.setColour(resultSuccessInk(tm.isDark()));
        g.drawText(juce::String(result.actualTrack), bounds, juce::Justification::centredLeft);
    }
    else
    {
        // Show what user guessed vs actual
        g.setColour(resultErrorInk(tm.isDark()));
        juce::String guessText = result.guessedTrack.empty() ? "No guess" : juce::String(result.guessedTrack);
        g.drawText(guessText + " -> " + juce::String(result.actualTrack), bounds, juce::Justification::centredLeft);
    }
}

void ResultsPanel::drawGuessScore(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();
    auto& fonts = FontManager::getInstance();
    bool isDark = tm.isDark();

    // Scores come from the manager's single-lock snapshot, not panel-local clicks.
    int roundCorrect = roundCorrectGuesses;
    int roundTotal = roundGuessAttempts;
    int roundPercentage = roundTotal > 0 ? (roundCorrect * 100 / roundTotal) : 0;

    // Score box background
    juce::Colour scoreBgColor = isDark ? juce::Colour(0xFF1E3A1E) : juce::Colour(0xFFE8F5E8);
    g.setColour(scoreBgColor);
    g.fillRoundedRectangle(bounds, 8.0f);

    // Border
    g.setColour(resultSuccessInk(isDark).withAlpha(0.5f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 8.0f, 1.0f);

    auto contentBounds = bounds.reduced(12.0f, 4.0f);

    if (showFinalResults && totalGuessAttempts > 0)
    {
        // Show final cumulative results
        int totalPercentage = totalGuessAttempts > 0 ? (totalCorrectGuesses * 100 / totalGuessAttempts) : 0;

        // Title
        auto titleRow = contentBounds.removeFromTop(20.0f);
        g.setColour(resultSuccessInk(isDark));
        g.setFont(fonts.getBold(13.0f));
        g.drawText("Final Score", titleRow, juce::Justification::centred);

        // Total score
        auto scoreRow = contentBounds;
        g.setColour(resultScoreInk(isDark));
        g.setFont(fonts.getBold(18.0f));
        juce::String totalScoreText = juce::String(totalCorrectGuesses) + "/" + juce::String(totalGuessAttempts) +
                                      " (" + juce::String(totalPercentage) + "%)";
        g.drawText(totalScoreText, scoreRow, juce::Justification::centred);
    }
    else
    {
        // Show round score with countdown
        auto scoreRow = contentBounds;

        // Round indicator
        juce::String roundLabel = "Round " + juce::String(currentRoundNumber) + "/" + juce::String(totalRounds) + ": ";

        g.setColour(tm.getColour(ColourId::TextSecondary));
        g.setFont(fonts.getMedium(13.0f));
        float labelWidth = 80.0f;
        g.drawText(roundLabel, scoreRow.removeFromLeft(labelWidth), juce::Justification::centredRight);

        // Score
        g.setColour(resultScoreInk(isDark));
        g.setFont(fonts.getBold(15.0f));
        juce::String scoreText = juce::String(roundCorrect) + "/" + juce::String(roundTotal) +
                                 " (" + juce::String(roundPercentage) + "%)";

        // If countdown is active, show it after the score
        if (countdownSeconds > 0)
        {
            scoreText += "  [" + juce::String(countdownSeconds) + "]";
        }

        g.drawText(scoreText, scoreRow, juce::Justification::centredLeft);
    }
}

//==============================================================================
// Q&A mode drawing

void ResultsPanel::drawQAResults(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    float rowHeight = 28.0f;

    for (const auto& result : qaResults)
    {
        auto rowBounds = bounds.removeFromTop(rowHeight);
        drawQARow(g, rowBounds, result);
        bounds.removeFromTop(4.0f);
    }

    bounds.removeFromTop(12.0f);  // Gap

    // Score summary
    drawQAScore(g, bounds.removeFromTop(24.0f));
}

void ResultsPanel::drawQARow(juce::Graphics& g, juce::Rectangle<float> bounds,
                              const QAResult& result)
{
    auto& tm = ThemeManager::getInstance();
    auto& fonts = FontManager::getInstance();

    // Correct/wrong icon
    auto iconArea = bounds.removeFromLeft(20.0f);
    g.setFont(fonts.getRegular(16.0f));
    if (result.wasCorrect)
    {
        g.setColour(resultSuccessInk(tm.isDark()));
        g.drawText(juce::String::fromUTF8("✓"), iconArea, juce::Justification::centred);
    }
    else
    {
        g.setColour(resultErrorInk(tm.isDark()));
        g.drawText(juce::String::fromUTF8("✗"), iconArea, juce::Justification::centred);
    }

    bounds.removeFromLeft(4.0f);

    // Plugin name
    g.setColour(tm.getColour(ColourId::TextPrimary));
    g.setFont(fonts.getMedium(13.0f));
    auto nameArea = bounds.removeFromLeft(bounds.getWidth() - 60.0f);
    g.drawText(juce::String(result.pluginName), nameArea, juce::Justification::centredLeft);

    // Card position
    g.setColour(tm.getColour(ColourId::TextMuted));
    g.drawText("Card " + juce::String(result.cardPosition + 1), bounds, juce::Justification::centredRight);
}

void ResultsPanel::drawQAScore(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();
    auto& fonts = FontManager::getInstance();

    int correct = 0;
    for (const auto& result : qaResults)
    {
        if (result.wasCorrect)
            correct++;
    }

    int total = static_cast<int>(qaResults.size());
    int percentage = total > 0 ? (correct * 100 / total) : 0;

    juce::String scoreText = "Score: " + juce::String(correct) + "/" + juce::String(total) +
                             " (" + juce::String(percentage) + "%)";

    g.setColour(resultScoreInk(tm.isDark()));
    g.setFont(fonts.getBold(16.0f));
    g.drawText(scoreText, bounds, juce::Justification::centred);
}

//==============================================================================
void ResultsPanel::resized()
{
    auto bounds = getLocalBounds();
    auto contentBounds = bounds.reduced(20);
    contentBounds.removeFromTop(28 + 8); // Fixed heading and gap.
    viewport->setBounds(contentBounds);
    updateContentLayout();
}

int ResultsPanel::measuredContentHeight() const
{
    const auto rowHeight = [this]() -> int
    {
        switch (currentMode)
        {
            case blindcard::RatingMode::Stars:
                return static_cast<int>(starsResults.size()) * 32
                     + juce::jmax(0, static_cast<int>(starsResults.size()) - 1) * 4;
            case blindcard::RatingMode::Guess:
            {
                const int rows = static_cast<int>(guessResults.size());
                const int list = rows * 32 + juce::jmax(0, rows - 1) * 3;
                return list + (guessResultsVisible ? 12 + (showFinalResults ? 60 : 40) : 48);
            }
            case blindcard::RatingMode::QA:
                return static_cast<int>(qaResults.size()) * 28
                     + static_cast<int>(qaResults.size()) * 4 + 12 + 32;
        }
        return 0;
    }();
    return juce::jmax(viewport->getHeight(), rowHeight);
}

void ResultsPanel::updateContentLayout(bool resetScroll)
{
    if (!viewport || !contentComponent || !submitButton) return;
    const int oldScroll = viewport->getViewPositionY();
    const int height = juce::jmax(1, measuredContentHeight());
    contentComponent->setSize(juce::jmax(1, viewport->getWidth() - 12), height);
    submitButton->setBounds(0, height - 40, contentComponent->getWidth(), 32);
    submitButton->setVisible(currentMode == blindcard::RatingMode::Guess && !guessResultsVisible);
    viewport->setViewPosition(0, resetScroll ? 0 : oldScroll);
    contentComponent->repaint();
}

} // namespace BlindCard
