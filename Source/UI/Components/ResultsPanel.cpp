/*
  ==============================================================================

    ResultsPanel.cpp
    Created: 2026-01-19
    Author:  BlindCard

  ==============================================================================
*/

#include "ResultsPanel.h"
#include <algorithm>

namespace BlindCard
{

//==============================================================================
ResultsPanel::ResultsPanel()
{
    ThemeManager::getInstance().addChangeListener(this);

    // Create submit button for Guess mode
    submitButton = std::make_unique<juce::TextButton>("SUBMIT GUESSES");
    submitButton->onClick = [this]()
    {
        if (onSubmitGuesses)
            onSubmitGuesses();
    };
    addChildComponent(*submitButton);  // Hidden by default
}

ResultsPanel::~ResultsPanel()
{
    ThemeManager::getInstance().removeChangeListener(this);
}

//==============================================================================
void ResultsPanel::setMode(blindcard::RatingMode mode)
{
    currentMode = mode;
    submitButton->setVisible(mode == blindcard::RatingMode::Guess && !guessResultsVisible);
    repaint();
}

//==============================================================================
void ResultsPanel::setStarsResults(const std::vector<StarsResult>& results)
{
    starsResults = results;

    // Sort by rating (highest first)
    std::sort(starsResults.begin(), starsResults.end(),
              [](const StarsResult& a, const StarsResult& b)
              {
                  return a.rating > b.rating;
              });

    repaint();
}

void ResultsPanel::setGuessResults(const std::vector<GuessResult>& results)
{
    guessResults = results;
    repaint();
}

void ResultsPanel::setSubmitEnabled(bool enabled)
{
    submitEnabled = enabled;
    submitButton->setEnabled(enabled);
}

void ResultsPanel::setGuessResultsVisible(bool showResults)
{
    guessResultsVisible = showResults;
    submitButton->setVisible(!showResults && currentMode == blindcard::RatingMode::Guess);
    repaint();
}

void ResultsPanel::setQAResults(const std::vector<QAResult>& results)
{
    qaResults = results;
    repaint();
}

void ResultsPanel::clearResults()
{
    starsResults.clear();
    guessResults.clear();
    qaResults.clear();
    guessResultsVisible = false;
    repaint();
}

//==============================================================================
void ResultsPanel::changeListenerCallback(juce::ChangeBroadcaster*)
{
    // Update button colors for theme
    auto& tm = ThemeManager::getInstance();
    submitButton->setColour(juce::TextButton::buttonColourId, tm.getColour(ColourId::Primary));
    submitButton->setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    repaint();
}

//==============================================================================
void ResultsPanel::paint(juce::Graphics& g)
{
    auto& tm = ThemeManager::getInstance();
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(tm.getColour(ColourId::Surface));
    g.fillRoundedRectangle(bounds, 8.0f);

    // Content area
    auto contentBounds = bounds.reduced(12.0f);

    // Header
    auto headerBounds = contentBounds.removeFromTop(28.0f);
    drawSectionHeader(g, headerBounds);

    contentBounds.removeFromTop(8.0f);  // Gap

    // Mode-specific content
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
                drawEmptyState(g, contentBounds);
            else if (guessResultsVisible)
                drawGuessResults(g, contentBounds);
            else
                drawGuessPending(g, contentBounds);
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

    g.setColour(tm.getColour(ColourId::TextSecondary));
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText("RESULTS", bounds, juce::Justification::centredLeft);
}

void ResultsPanel::drawEmptyState(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();

    g.setColour(tm.getColour(ColourId::TextMuted));
    g.setFont(juce::Font(12.0f, juce::Font::italic));

    juce::String message;
    switch (currentMode)
    {
        case blindcard::RatingMode::Stars:
            message = "Rate the cards to see results";
            break;
        case blindcard::RatingMode::Guess:
            message = "Make your guesses to see results";
            break;
        case blindcard::RatingMode::QA:
            message = "Answer questions to see results";
            break;
    }

    g.drawText(message, bounds, juce::Justification::centred);
}

//==============================================================================
// Stars mode drawing

void ResultsPanel::drawStarsResults(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    float rowHeight = 24.0f;

    for (const auto& result : starsResults)
    {
        auto rowBounds = bounds.removeFromTop(rowHeight);
        drawStarsRow(g, rowBounds, result);
    }
}

void ResultsPanel::drawStarsRow(juce::Graphics& g, juce::Rectangle<float> bounds,
                                 const StarsResult& result)
{
    auto& tm = ThemeManager::getInstance();

    // Stars (5 total)
    auto starsArea = bounds.removeFromLeft(80.0f);
    g.setFont(juce::Font(12.0f));

    for (int i = 0; i < 5; ++i)
    {
        bool filled = i < result.rating;
        g.setColour(filled ? tm.getColour(ColourId::StarFilled) : tm.getColour(ColourId::StarEmpty));
        auto starArea = starsArea.removeFromLeft(14.0f);
        g.drawText(filled ? juce::String::fromUTF8("★") : juce::String::fromUTF8("☆"),
                   starArea, juce::Justification::centred);
    }

    bounds.removeFromLeft(8.0f);  // Gap

    // Track name
    g.setColour(tm.getColour(ColourId::TextPrimary));
    g.setFont(juce::Font(12.0f));
    auto nameArea = bounds.removeFromLeft(bounds.getWidth() - 50.0f);
    g.drawText(juce::String(result.trackName), nameArea, juce::Justification::centredLeft);

    // Position
    g.setColour(tm.getColour(ColourId::TextMuted));
    g.setFont(juce::Font(10.0f));
    g.drawText("(Pos " + juce::String(result.cardPosition + 1) + ")", bounds, juce::Justification::centredRight);
}

//==============================================================================
// Guess mode drawing

void ResultsPanel::drawGuessResults(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    float rowHeight = 24.0f;

    // Results list
    for (const auto& result : guessResults)
    {
        auto rowBounds = bounds.removeFromTop(rowHeight);
        drawGuessRow(g, rowBounds, result);
    }

    bounds.removeFromTop(12.0f);  // Gap

    // Score summary
    drawGuessScore(g, bounds.removeFromTop(24.0f));
}

void ResultsPanel::drawGuessPending(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();

    float rowHeight = 24.0f;

    for (size_t i = 0; i < guessResults.size(); ++i)
    {
        auto rowBounds = bounds.removeFromTop(rowHeight);
        auto& result = guessResults[i];

        // Card number
        g.setColour(tm.getColour(ColourId::TextSecondary));
        g.setFont(juce::Font(12.0f));
        g.drawText("Card " + juce::String(result.cardPosition + 1) + ":",
                   rowBounds.removeFromLeft(60.0f), juce::Justification::centredLeft);

        rowBounds.removeFromLeft(8.0f);

        // Guessed track (or "Not guessed")
        if (!result.guessedTrack.empty())
        {
            g.setColour(tm.getColour(ColourId::TextPrimary));
            g.drawText(juce::String(result.guessedTrack), rowBounds, juce::Justification::centredLeft);
        }
        else
        {
            g.setColour(tm.getColour(ColourId::TextMuted));
            g.setFont(juce::Font(12.0f, juce::Font::italic));
            g.drawText("Not guessed", rowBounds, juce::Justification::centredLeft);
        }
    }
}

void ResultsPanel::drawGuessRow(juce::Graphics& g, juce::Rectangle<float> bounds,
                                 const GuessResult& result)
{
    auto& tm = ThemeManager::getInstance();

    // Correct/wrong icon
    auto iconArea = bounds.removeFromLeft(20.0f);
    g.setFont(juce::Font(14.0f));
    if (result.isCorrect)
    {
        g.setColour(tm.getColour(ColourId::Success));
        g.drawText(juce::String::fromUTF8("✓"), iconArea, juce::Justification::centred);
    }
    else
    {
        g.setColour(tm.getColour(ColourId::Error));
        g.drawText(juce::String::fromUTF8("✗"), iconArea, juce::Justification::centred);
    }

    bounds.removeFromLeft(4.0f);

    // Card number
    g.setColour(tm.getColour(ColourId::TextSecondary));
    g.setFont(juce::Font(12.0f));
    g.drawText("Card " + juce::String(result.cardPosition + 1) + ":",
               bounds.removeFromLeft(55.0f), juce::Justification::centredLeft);

    bounds.removeFromLeft(4.0f);

    // Result text
    if (result.isCorrect)
    {
        g.setColour(tm.getColour(ColourId::Success));
        g.drawText("Correct", bounds, juce::Justification::centredLeft);
    }
    else
    {
        g.setColour(tm.getColour(ColourId::TextPrimary));
        g.drawText("Was: " + juce::String(result.actualTrack), bounds, juce::Justification::centredLeft);
    }
}

void ResultsPanel::drawGuessScore(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();

    int correct = 0;
    for (const auto& result : guessResults)
    {
        if (result.isCorrect)
            correct++;
    }

    int total = static_cast<int>(guessResults.size());
    int percentage = total > 0 ? (correct * 100 / total) : 0;

    juce::String scoreText = "Score: " + juce::String(correct) + "/" + juce::String(total) +
                             " (" + juce::String(percentage) + "%)";

    g.setColour(tm.getColour(ColourId::Accent));
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText(scoreText, bounds, juce::Justification::centred);
}

//==============================================================================
// Q&A mode drawing

void ResultsPanel::drawQAResults(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    float rowHeight = 24.0f;

    for (const auto& result : qaResults)
    {
        auto rowBounds = bounds.removeFromTop(rowHeight);
        drawQARow(g, rowBounds, result);
    }

    bounds.removeFromTop(12.0f);  // Gap

    // Score summary
    drawQAScore(g, bounds.removeFromTop(24.0f));
}

void ResultsPanel::drawQARow(juce::Graphics& g, juce::Rectangle<float> bounds,
                              const QAResult& result)
{
    auto& tm = ThemeManager::getInstance();

    // Correct/wrong icon
    auto iconArea = bounds.removeFromLeft(20.0f);
    g.setFont(juce::Font(14.0f));
    if (result.wasCorrect)
    {
        g.setColour(tm.getColour(ColourId::Success));
        g.drawText(juce::String::fromUTF8("✓"), iconArea, juce::Justification::centred);
    }
    else
    {
        g.setColour(tm.getColour(ColourId::Error));
        g.drawText(juce::String::fromUTF8("✗"), iconArea, juce::Justification::centred);
    }

    bounds.removeFromLeft(4.0f);

    // Plugin name
    g.setColour(tm.getColour(ColourId::TextPrimary));
    g.setFont(juce::Font(12.0f));
    auto nameArea = bounds.removeFromLeft(bounds.getWidth() - 60.0f);
    g.drawText(juce::String(result.pluginName), nameArea, juce::Justification::centredLeft);

    // Card position
    g.setColour(tm.getColour(ColourId::TextMuted));
    g.drawText("Card " + juce::String(result.cardPosition + 1), bounds, juce::Justification::centredRight);
}

void ResultsPanel::drawQAScore(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto& tm = ThemeManager::getInstance();

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

    g.setColour(tm.getColour(ColourId::Accent));
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText(scoreText, bounds, juce::Justification::centred);
}

//==============================================================================
void ResultsPanel::resized()
{
    auto bounds = getLocalBounds();
    auto contentBounds = bounds.reduced(12);

    // Submit button at bottom (Guess mode only)
    if (currentMode == blindcard::RatingMode::Guess && !guessResultsVisible)
    {
        auto buttonArea = contentBounds.removeFromBottom(36);
        submitButton->setBounds(buttonArea.reduced(0, 4));
    }
}

} // namespace BlindCard
