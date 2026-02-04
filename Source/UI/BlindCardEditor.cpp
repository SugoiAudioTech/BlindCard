/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    BlindCardEditor.cpp
    Created: 2026-01-19
    Author:  BlindCard

  ==============================================================================
*/

#include "BlindCardEditor.h"
#include "../PluginProcessor.h"
#include "Theme/FontManager.h"
#include <algorithm>

namespace BlindCard
{

//==============================================================================
// PresetUILookAndFeel Implementation
//==============================================================================
PresetUILookAndFeel::PresetUILookAndFeel()
{
    updateColours(true);  // Default to dark theme
}

void PresetUILookAndFeel::updateColours(bool isDark)
{
    if (isDark)
    {
        // Dark theme colors
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF2A2A2A));
        setColour(juce::ComboBox::textColourId, juce::Colours::white);
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFF3D3D3D));
        setColour(juce::ComboBox::arrowColourId, juce::Colours::white);
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xFF2A2A2A));
        setColour(juce::PopupMenu::textColourId, juce::Colours::white);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xFF4A4A4A));
    }
    else
    {
        // Light theme colors
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFFF5F5F5));
        setColour(juce::ComboBox::textColourId, juce::Colour(0xFF333333));
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFFCCCCCC));
        setColour(juce::ComboBox::arrowColourId, juce::Colour(0xFF666666));
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xFFF5F5F5));
        setColour(juce::PopupMenu::textColourId, juce::Colour(0xFF333333));
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xFFE0E0E0));
    }
}

juce::Font PresetUILookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    // Readable font for preset names
    return FontManager::getInstance().getMedium(14.0f);
}

juce::Font PresetUILookAndFeel::getTextButtonFont(juce::TextButton&, int)
{
    // Display font for action buttons
    return FontManager::getInstance().getBebasNeue(14.0f);
}

juce::Font PresetUILookAndFeel::getPopupMenuFont()
{
    // Readable font for dropdown menu
    return FontManager::getInstance().getMedium(14.0f);
}

//==============================================================================
BlindCardEditor::BlindCardEditor(BlindCardProcessor& processor)
    : AudioProcessorEditor(processor)
    , processorRef(processor)
{
    // Subscribe to manager and theme changes
    manager->addChangeListener(this);
    ThemeManager::getInstance().addChangeListener(this);

    // Create UI components BEFORE setting size (setResizeLimits triggers resized())
    headerBar = std::make_unique<HeaderBar>();
    headerBar->onSettingsClicked = [this]() { onSettingsClicked(); };
    headerBar->onInfoClicked = [this]() { onInfoClicked(); };
    headerBar->onThemeToggled = [this]() { onThemeToggled(); };
    addAndMakeVisible(*headerBar);

    modeSelector = std::make_unique<ModeSelector>();
    modeSelector->onModeChanged = [this](blindcard::RatingMode mode) { onModeChanged(mode); };
    addAndMakeVisible(*modeSelector);

    questionBanner = std::make_unique<QuestionBanner>();
    addAndMakeVisible(*questionBanner);

    pokerTable = std::make_unique<PokerTable>();
    pokerTable->onCardClicked = [this](int idx) { onCardClicked(idx); };
    pokerTable->onCardRatingChanged = [this](int idx, int r) { onCardRatingChanged(idx, r); };
    pokerTable->onCardGuessChanged = [this](int idx, int t) { onCardGuessChanged(idx, t); };
    pokerTable->onCardQASelected = [this](int idx) { onCardQASelected(idx); };
    addAndMakeVisible(*pokerTable);

    controlPanel = std::make_unique<ControlPanel>();
    controlPanel->onShuffleClicked = [this]() { onShuffleClicked(); };
    controlPanel->onRevealClicked = [this]() { onRevealClicked(); };
    controlPanel->onResetClicked = [this]() { onResetClicked(); };
    controlPanel->onNextRoundClicked = [this]() { onNextRoundClicked(); };
    controlPanel->onRoundsChanged = [this](int r) { onRoundsChanged(r); };
    controlPanel->onAutoGainChanged = [this](bool e) { onAutoGainChanged(e); };
    controlPanel->onQAQuestionsChanged = [this](int q) { onQAQuestionsChanged(q); };
    addAndMakeVisible(*controlPanel);

    resultsPanel = std::make_unique<ResultsPanel>();
    resultsPanel->onSubmitGuesses = [this]() { onSubmitGuesses(); };
    addAndMakeVisible(*resultsPanel);

    // Quick Start Guide (hidden by default, shown when clicking ? button)
    quickStartGuide = std::make_unique<QuickStartGuide>();

    // Settings Panel (hidden by default, shown when clicking gear button)
    settingsPanel = std::make_unique<SettingsPanel>();

    // Set window constraints AFTER components are created
    // (setResizeLimits triggers resized() which needs valid component pointers)
    setResizable(true, true);
    setResizeLimits(kMinWidth, kMinHeight, kMaxWidth, kMaxHeight);
    setSize(kMinWidth, kMinHeight);

    // Check if running in Standalone mode
    isStandaloneMode = (processorRef.wrapperType == juce::AudioProcessor::wrapperType_Standalone);

    // Setup Standalone mode components (audio engine, transport bar, card count control)
    if (isStandaloneMode)
    {
        setupStandaloneMode();
    }

    // Setup dev panel (hidden by default, press 'D' to toggle)
    setupDevPanel();

    // Initial state sync
    updateFromManager();

    // If Level-Match is enabled by default, start calibration
    if (manager->isLevelMatchEnabled() && manager->getPhase() == blindcard::GamePhase::Setup)
        manager->startCalibration();

    // Start update timer (for connection status, etc.)
    startTimerHz(30);

    // Add keyboard listener for dev mode toggle and card navigation
    setWantsKeyboardFocus(true);
    addKeyListener(this);

    // Ensure mouse clicks can be captured (for obtaining keyboard focus)
    setInterceptsMouseClicks(true, true);
}

BlindCardEditor::~BlindCardEditor()
{
    removeKeyListener(this);
    stopTimer();
    manager->removeChangeListener(this);
    ThemeManager::getInstance().removeChangeListener(this);

    // Clear LookAndFeel before destruction to avoid crashes
    if (presetComboBox) presetComboBox->setLookAndFeel(nullptr);
    if (savePresetButton) savePresetButton->setLookAndFeel(nullptr);
    if (deletePresetButton) deletePresetButton->setLookAndFeel(nullptr);
    if (importFilesButton) importFilesButton->setLookAndFeel(nullptr);
}

//==============================================================================
void BlindCardEditor::paint(juce::Graphics& g)
{
    auto& tm = ThemeManager::getInstance();
    g.fillAll(tm.getColour(ColourId::Background));

    // Draw dev panel background if visible
    if (devPanel && devPanelVisible)
    {
        auto devBounds = devPanel->getBounds().toFloat();
        g.setColour(juce::Colour(0xDD222222));  // Semi-transparent dark
        g.fillRect(devBounds);
        g.setColour(juce::Colour(0xFF4CAF50));  // Green accent line
        g.drawLine(devBounds.getX(), devBounds.getBottom(),
                   devBounds.getRight(), devBounds.getBottom(), 2.0f);
    }
}

void BlindCardEditor::resized()
{
    // Ensure keyboard focus is maintained after window resize (fixes arrow key not working after dragging window)
    // Use callAsync for delayed execution, ensure focus is grabbed after layout is complete
    // Use SafePointer to prevent dangling pointer after component is destroyed
    juce::Component::SafePointer<BlindCardEditor> safeThis(this);
    juce::MessageManager::callAsync([safeThis]()
    {
        if (safeThis != nullptr && safeThis->isShowing() && !safeThis->hasKeyboardFocus(true))
        {
            safeThis->grabKeyboardFocus();
        }
    });

    auto bounds = getLocalBounds();

    // Dev panel (overlay style, toggle with 'D' key)
    if (devPanel && devPanelVisible)
    {
        auto devBounds = bounds.removeFromTop(36);
        devPanel->setBounds(devBounds);

        // Layout dev panel children
        auto contentBounds = devBounds.reduced(50, 4);  // Leave space for DEV button
        trackCountLabel->setBounds(contentBounds.removeFromLeft(100));
        contentBounds.removeFromLeft(8);
        trackCountSlider->setBounds(contentBounds.removeFromLeft(150));
        contentBounds.removeFromLeft(12);
        applyButton->setBounds(contentBounds.removeFromLeft(80));
    }

    // Header at top
    auto headerArea = bounds.removeFromTop(kHeaderHeight);

    // In Standalone mode, place TransportBar in header CENTER (between logo and settings buttons)
    if (isStandaloneMode && transportBar)
    {
        // TransportBar centered in header
        int transportWidth = 300;
        auto transportBounds = headerArea.withSizeKeepingCentre(transportWidth, 40);
        transportBar->setBounds(transportBounds);
    }

    // In Standalone mode, place preset controls below header (above mode selector)
    if (isStandaloneMode && presetComboBox)
    {
        // Row 1: Preset controls on left, Now Playing centered
        auto presetRow = bounds.removeFromTop(32);
        presetRow = presetRow.reduced(16, 2);

        // Left side: [ComboBox] [Save] [Delete]
        auto leftControls = presetRow.removeFromLeft(340);
        presetComboBox->setBounds(leftControls.removeFromLeft(200));
        leftControls.removeFromLeft(8);
        savePresetButton->setBounds(leftControls.removeFromLeft(60));
        leftControls.removeFromLeft(4);
        deletePresetButton->setBounds(leftControls.removeFromLeft(60));

        // Center: [Now Playing] [TrackName] - centered in FULL window width
        if (nowPlayingLabel && nowPlayingTrackName)
        {
            // Calculate total width needed for Now Playing display
            int nowPlayingWidth = 100 + 10 + 250;  // label + gap + track name
            int windowCenterX = getWidth() / 2;
            int nowPlayingX = windowCenterX - nowPlayingWidth / 2;

            // Ensure it doesn't overlap with left controls (340px + 20px margin)
            int minX = 360;
            nowPlayingX = juce::jmax(nowPlayingX, minX);

            auto nowPlayingArea = juce::Rectangle<int>(nowPlayingX, presetRow.getY(), nowPlayingWidth, presetRow.getHeight());

            nowPlayingLabel->setBounds(nowPlayingArea.removeFromLeft(100));
            nowPlayingArea.removeFromLeft(10);
            nowPlayingTrackName->setBounds(nowPlayingArea);
        }

        // Row 2: Import Files below preset combo
        if (importFilesButton)
        {
            auto importRow = bounds.removeFromTop(28);
            importRow = importRow.reduced(16, 2);
            importFilesButton->setBounds(importRow.removeFromLeft(110));
        }
    }

    headerBar->setBounds(headerArea);

    // Mode selector below header
    auto modeSelectorArea = bounds.removeFromTop(kModeSelectorHeight);

    // Mode selector always centered in FULL width (before removing CardCountControl)
    auto selectorBounds = modeSelectorArea.withSizeKeepingCentre(280, 40);
    modeSelector->setBounds(selectorBounds);

    // In Standalone mode, place CardCountControl on the right side (after centering mode selector)
    if (isStandaloneMode && cardCountControl)
    {
        auto cardCountArea = modeSelectorArea.removeFromRight(140);
        cardCountControl->setBounds(cardCountArea.reduced(8, 8));
    }

    // Question banner area - always reserve space to prevent layout jumping
    auto bannerArea = bounds.removeFromTop(kQuestionBannerHeight);
    questionBanner->setBounds(bannerArea.reduced(100, 4));

    // Only show banner in Q&A mode AND when game has started (after shuffle)
    auto currentMode = manager->getRatingMode();
    auto currentPhase = manager->getPhase();
    bool showBanner = (currentMode == blindcard::RatingMode::QA) &&
                      (currentPhase == blindcard::GamePhase::BlindTesting ||
                       currentPhase == blindcard::GamePhase::Revealed);
    questionBanner->setVisible(showBanner);

    // Main content area with padding
    auto mainArea = bounds.reduced(16, 8);

    // Bottom panel area (Control Panel + Results Panel side by side)
    constexpr int kBottomPanelHeight = ControlPanel::kMinHeight;
    auto bottomArea = mainArea.removeFromBottom(kBottomPanelHeight);
    mainArea.removeFromBottom(12);  // Gap between table and bottom panels

    // Split bottom area: Control Panel (left) and Results Panel (right)
    auto controlPanelArea = bottomArea.removeFromLeft(bottomArea.getWidth() / 2 - 8);
    bottomArea.removeFromLeft(16);  // Gap between panels
    auto resultsPanelArea = bottomArea;

    controlPanel->setBounds(controlPanelArea);
    resultsPanel->setBounds(resultsPanelArea);

    // Poker table fills remaining main area (top portion)
    pokerTable->setBounds(mainArea);
}

//==============================================================================
void BlindCardEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    // Handle theme changes
    if (source == &ThemeManager::getInstance())
    {
        juce::MessageManager::callAsync([this]()
        {
            // Update preset UI colors for current theme
            if (presetUILookAndFeel)
            {
                presetUILookAndFeel->updateColours(ThemeManager::getInstance().isDark());
                if (presetComboBox) presetComboBox->repaint();
                if (savePresetButton) savePresetButton->repaint();
                if (deletePresetButton) deletePresetButton->repaint();
                if (importFilesButton) importFilesButton->repaint();
            }
            repaint();
        });
        return;
    }

    // Called when BlindCardManager state changes
    juce::MessageManager::callAsync([this]()
    {
        updateFromManager();
    });
}

void BlindCardEditor::timerCallback()
{
    // Update Standalone mode transport bar
    if (isStandaloneMode)
    {
        updateTransportFromEngine();
    }

    // Update track info display (RMS + track name)
    float rmsDb = manager->getCurrentPlayingRMSdB();
    juce::String trackName = manager->getCurrentPlayingTrackName();
    headerBar->setCurrentTrackInfo(rmsDb, trackName);

    // Update calibration status display (always update when Level-Match is enabled)
    if (manager->isLevelMatchEnabled())
    {
        controlPanel->setCalibrationStatus(
            manager->isCalibrating(),
            manager->isCalibrated(),
            manager->getCalibrationTimeRemaining());
    }

    // Handle Q&A countdown (tick every 1 second)
    auto mode = manager->getRatingMode();
    const auto& qaState = manager->getQAState();

    if (mode == blindcard::RatingMode::QA && qaState.isShowingAnswer)
    {
        auto now = juce::Time::currentTimeMillis();
        if (now - lastCountdownTick >= 1000)  // 1 second interval
        {
            lastCountdownTick = now;
            manager->tickQACountdown();
        }
    }

    // Handle Guess mode countdown and auto-advance after reveal
    if (autoAdvanceStartTime > 0 && mode == blindcard::RatingMode::Guess)
    {
        auto now = juce::Time::currentTimeMillis();
        auto elapsed = now - autoAdvanceStartTime;

        // Calculate remaining seconds (3, 2, 1, 0)
        int newCountdown = kAutoAdvanceSeconds - static_cast<int>(elapsed / 1000);
        if (newCountdown < 0) newCountdown = 0;

        // Update countdown display if changed
        if (newCountdown != autoAdvanceCountdown)
        {
            autoAdvanceCountdown = newCountdown;
            resultsPanel->setCountdown(autoAdvanceCountdown);
        }

        // When countdown reaches 0, advance to next round
        if (autoAdvanceCountdown == 0 && elapsed >= kAutoAdvanceSeconds * 1000)
        {
            autoAdvanceStartTime = 0;  // Clear timer
            resultsPanel->setCountdown(0);

            // Check if we're still in Revealed phase (user might have clicked manually)
            if (manager->getPhase() == blindcard::GamePhase::Revealed)
            {
                int currentRound = manager->getCurrentRound() + 1;
                int totalRounds = manager->getTotalRounds();

                if (currentRound >= totalRounds)
                {
                    // Last round - show final results (already showing)
                    resultsPanel->setShowFinalResults(true);
                }
                else
                {
                    // Go to next round - this will shuffle and flip cards
                    resultsPanel->setGuessResultsVisible(false);
                    resultsPanel->setShowFinalResults(false);
                    manager->nextRound();
                }
            }
        }
    }
}

//==============================================================================
void BlindCardEditor::updateFromManager()
{
    // Sync card count
    auto cards = manager->getCards();
    int cardCount = cards.size();

    // Always sync card count (min 2, max 8)
    if (cardCount >= blindcard::GameState::MinCards)
    {
        pokerTable->setCardCount(cardCount);
    }
    else if (cardCount > 0)
    {
        pokerTable->setCardCount(blindcard::GameState::MinCards);
    }

    // Auto-resize window for 5+ cards (two-row layout needs more height)
    if (cardCount > 4 && getHeight() < 850)
    {
        setSize(getWidth(), 850);
    }

    // Sync track names
    auto trackNames = getTrackNames();
    controlPanel->setTracks(trackNames);
    pokerTable->setTracks(trackNames);

    // Sync mode
    auto mode = manager->getRatingMode();
    modeSelector->setMode(mode);
    pokerTable->setMode(mode);
    resultsPanel->setMode(mode);

    // Sync Q&A mode state to control panel
    bool isQAMode = (mode == blindcard::RatingMode::QA);
    controlPanel->setQAMode(isQAMode);
    controlPanel->setQAQuestions(manager->getQAQuestionCount());

    // Sync phase
    auto phase = manager->getPhase();
    pokerTable->setPhase(phase);
    resultsPanel->setPhase(phase);

    // Sync rounds info
    controlPanel->setCurrentRound(manager->getCurrentRound() + 1);  // Convert 0-based to 1-based
    controlPanel->setRounds(manager->getTotalRounds());

    // Sync Level-Match / Auto Gain state
    controlPanel->setAutoGain(manager->isLevelMatchEnabled());
    controlPanel->setCalibrationStatus(manager->isCalibrating(), manager->isCalibrated(),
                                        manager->getCalibrationTimeRemaining());

    // Update card states
    updateCardStates();

    // Update mode-specific UI
    updateModeUI();
    updatePhaseUI();
    updateResultsPanel();

    repaint();
}

void BlindCardEditor::updateCardStates()
{
    auto cards = manager->getCards();
    auto selectedId = manager->getSelectedCardId();
    int correctAnswerCardId = manager->getQACorrectAnswerCardId();

    for (size_t i = 0; i < cards.size(); ++i)
    {
        const auto& slot = cards[static_cast<int>(i)];

        // Use displayPosition to determine card's position on the table
        // This way cards appear at different positions after shuffle
        auto* card = pokerTable->getCard(slot.displayPosition);
        if (card == nullptr)
            continue;

        CardData data;
        // The letter shown on card (A, B, C...) reflects its position on the table
        data.position = slot.displayPosition;

        // In standalone mode, preserve trackName if card has loaded audio file
        // (audio file name was set when file was dropped, don't overwrite with empty manager trackName)
        if (isStandaloneMode && card->hasAudioFileLoaded())
        {
            data.trackName = card->getCardData().trackName;
        }
        else
        {
            data.trackName = slot.realTrackName.toStdString();
        }

        // Show card FRONT in Setup and Revealed phases, show BACK only in BlindTesting
        // Exception: In Q&A mode, flip the correct answer card during countdown
        bool isCorrectAnswerCard = (slot.id == correctAnswerCardId);
        data.isRevealed = (manager->getPhase() != blindcard::GamePhase::BlindTesting)
                        || isCorrectAnswerCard;  // Flip correct answer during Q&A reveal
        data.isSelected = (slot.id == selectedId);
        data.isPlaying = (slot.id == selectedId);

        // Q&A mode: highlight correct answer card during reveal
        data.isCorrectAnswer = isCorrectAnswerCard;

        // Copy random card face value and suit
        data.cardValue = slot.cardValue;
        data.suitIndex = slot.suitIndex;

        // Get current round data
        int currentRound = manager->getCurrentRound();
        if (currentRound < slot.rounds.size())
        {
            data.rating = slot.rounds[currentRound].rating;
            data.guessedTrackIndex = slot.rounds[currentRound].guessedTrackId;
        }

        card->setCardData(data);
    }
}

void BlindCardEditor::updateModeUI()
{
    auto mode = manager->getRatingMode();
    auto phase = manager->getPhase();

    // Update question banner visibility
    bool showBanner = (mode == blindcard::RatingMode::QA) &&
                      (phase == blindcard::GamePhase::BlindTesting ||
                       phase == blindcard::GamePhase::Revealed);
    questionBanner->setVisible(showBanner);

    if (mode == blindcard::RatingMode::QA)
    {
        updateQAUI();
    }
}

void BlindCardEditor::updatePhaseUI()
{
    auto phase = manager->getPhase();

    // Update button states based on phase
    switch (phase)
    {
        case blindcard::GamePhase::Setup:
            // Use card count (not registered instances) for shuffle - allows standalone testing
            controlPanel->setShuffleEnabled(manager->getCards().size() >= blindcard::GameState::MinCards);
            controlPanel->setRevealEnabled(false);
            controlPanel->setResetEnabled(false);
            controlPanel->setNextRoundEnabled(false);
            modeSelector->setLocked(false);
            break;

        case blindcard::GamePhase::BlindTesting:
        {
            controlPanel->setShuffleEnabled(false);
            controlPanel->setResetEnabled(true);
            modeSelector->setLocked(true);

            // Stars/Guess mode: NEXT available during testing, REVEAL only on last round
            // Q&A mode: No NEXT (handled separately), REVEAL after all questions
            auto mode = manager->getRatingMode();
            bool isLastRound = manager->getCurrentRound() >= manager->getTotalRounds() - 1;

            if (mode == blindcard::RatingMode::QA)
            {
                controlPanel->setRevealEnabled(false);  // Q&A handles reveal automatically
                controlPanel->setNextRoundEnabled(false);
            }
            else
            {
                // Stars/Guess: Can go NEXT if not last round, REVEAL if last round
                controlPanel->setNextRoundEnabled(!isLastRound);
                controlPanel->setRevealEnabled(isLastRound);
            }
            break;
        }

        case blindcard::GamePhase::Revealed:
        {
            controlPanel->setShuffleEnabled(false);
            controlPanel->setRevealEnabled(false);
            controlPanel->setResetEnabled(true);
            modeSelector->setLocked(true);

            // Enable NEXT if not the last round (to continue to next round)
            bool isLastRound = manager->getCurrentRound() >= manager->getTotalRounds() - 1;
            controlPanel->setNextRoundEnabled(!isLastRound);
            break;
        }
    }
}

void BlindCardEditor::updateQAUI()
{
    const auto& qaState = manager->getQAState();
    auto phase = manager->getPhase();

    // Check if Q&A is complete (all questions answered and revealed)
    if (phase == blindcard::GamePhase::Revealed)
    {
        // Count correct answers
        int correctCount = 0;
        for (bool answer : qaState.answers)
        {
            if (answer)
                correctCount++;
        }
        int totalCount = qaState.answers.size();

        // Show completion banner
        questionBanner->showCompletion(correctCount, totalCount);
        return;
    }

    // Update question banner
    auto questionTrack = manager->getCurrentQuestionTrackName();
    questionBanner->setQuestion(questionTrack.toStdString());
    questionBanner->setProgress(qaState.currentQuestion + 1, manager->getQAMaxQuestions());

    // Show feedback if any
    switch (qaState.lastFeedback)
    {
        case blindcard::QAState::FeedbackState::None:
            questionBanner->clearFeedback();
            break;
        case blindcard::QAState::FeedbackState::Correct:
            questionBanner->showFeedback(QAFeedback::Correct);
            break;
        case blindcard::QAState::FeedbackState::Wrong:
            questionBanner->showFeedback(QAFeedback::Wrong);
            break;
    }

    // Handle countdown display
    if (qaState.isShowingAnswer)
    {
        // Initialize countdown tick timer if just started
        if (lastCountdownTick == 0)
            lastCountdownTick = juce::Time::currentTimeMillis();

        // Set countdown value
        questionBanner->setCountdown(qaState.countdownValue);

        // Set correct answer track name
        auto cards = manager->getCards();
        for (const auto& card : cards)
        {
            if (card.id == qaState.revealedTargetCardId)
            {
                questionBanner->setCorrectAnswer(card.realTrackName.toStdString());
                break;
            }
        }
    }
    else
    {
        // Reset countdown state
        lastCountdownTick = 0;
        questionBanner->setCountdown(0);
    }
}

void BlindCardEditor::updateResultsPanel()
{
    auto mode = manager->getRatingMode();
    auto phase = manager->getPhase();

    // Update round info for Guess mode
    if (mode == blindcard::RatingMode::Guess)
    {
        int currentRound = manager->getCurrentRound() + 1;  // 1-based for display
        int totalRounds = manager->getTotalRounds();
        resultsPanel->setRoundInfo(currentRound, totalRounds);

        // Auto-show results when phase is Revealed
        if (phase == blindcard::GamePhase::Revealed)
        {
            resultsPanel->setGuessResultsVisible(true);

            // Check if this is the last round for final results display
            bool isLastRound = (currentRound >= totalRounds);
            resultsPanel->setShowFinalResults(isLastRound);
        }
    }

    switch (mode)
    {
        case blindcard::RatingMode::Stars:
            buildStarsResults();
            break;
        case blindcard::RatingMode::Guess:
            buildGuessResults();
            break;
        case blindcard::RatingMode::QA:
            buildQAResults();
            break;
    }
}

//==============================================================================
// Event handlers - HeaderBar

void BlindCardEditor::onSettingsClicked()
{
    settingsPanel->showOverlay(this);
}

void BlindCardEditor::onInfoClicked()
{
    quickStartGuide->showOverlay(this);
}

void BlindCardEditor::onThemeToggled()
{
    // Toggle theme via ThemeManager
    ThemeManager::getInstance().toggleTheme();
}

//==============================================================================
// Event handlers - ModeSelector

void BlindCardEditor::onModeChanged(blindcard::RatingMode mode)
{
    manager->setRatingMode(mode);

    // In Q&A mode, sync Rounds setting to question count
    if (mode == blindcard::RatingMode::QA)
    {
        int rounds = controlPanel->getRounds();
        manager->setQAQuestionCount(rounds);
    }
}

//==============================================================================
// Event handlers - PokerTable

// Helper: Find card ID from table position
// tablePosition is the position on PokerTable (0, 1, 2, 3...)
// Need to find card where displayPosition == tablePosition
int BlindCardEditor::findCardIdAtTablePosition(int tablePosition)
{
    auto cards = manager->getCards();
    for (const auto& card : cards)
    {
        if (card.displayPosition == tablePosition)
            return card.id;
    }
    return -1;  // Not found
}

void BlindCardEditor::onCardClicked(int tablePosition)
{
    int cardId = findCardIdAtTablePosition(tablePosition);
    if (cardId >= 0)
    {
        manager->selectCard(cardId);

        // In Standalone mode, switch audio playback to clicked card
        if (isStandaloneMode && audioEngine)
        {
            audioEngine->switchToCard(tablePosition);
        }
    }
}

void BlindCardEditor::onCardRatingChanged(int tablePosition, int rating)
{
    int cardId = findCardIdAtTablePosition(tablePosition);
    if (cardId >= 0)
    {
        manager->rateCard(cardId, rating);
    }
}

void BlindCardEditor::onCardGuessChanged(int tablePosition, int trackIndex)
{
    int cardId = findCardIdAtTablePosition(tablePosition);
    if (cardId >= 0)
    {
        manager->setGuess(cardId, trackIndex);
    }
}

void BlindCardEditor::onCardQASelected(int tablePosition)
{
    int cardId = findCardIdAtTablePosition(tablePosition);
    if (cardId >= 0)
    {
        manager->submitQAAnswer(cardId);
    }
}

//==============================================================================
// Event handlers - ControlPanel

void BlindCardEditor::onShuffleClicked()
{
    // 如果 Level-Match 啟用且正在校準中，先鎖定校準結果
    if (manager->isLevelMatchEnabled() && manager->isCalibrating())
        manager->lockCalibration();

    manager->shuffle();
}

void BlindCardEditor::onRevealClicked()
{
    auto mode = manager->getRatingMode();

    // Handle Guess mode - show results with scoring
    if (mode == blindcard::RatingMode::Guess)
    {
        int currentRound = manager->getCurrentRound() + 1;  // 1-based for display
        int totalRounds = manager->getTotalRounds();

        resultsPanel->setRoundInfo(currentRound, totalRounds);
        resultsPanel->setGuessResultsVisible(true);

        // Build results first so we can accumulate them
        buildGuessResults();
        resultsPanel->accumulateGuessResults();

        // Check if this is the last round
        bool isLastRound = (currentRound >= totalRounds);
        resultsPanel->setShowFinalResults(isLastRound);
    }

    manager->reveal();
    pokerTable->revealAllCards(100);  // Staggered 100ms animation
}

void BlindCardEditor::onResetClicked()
{
    manager->reset();

    // Cancel any pending auto-advance
    autoAdvanceStartTime = 0;
    autoAdvanceCountdown = 0;
    resultsPanel->setCountdown(0);

    // Reset results panel completely
    resultsPanel->clearResults();
    resultsPanel->resetCumulativeTracking();
    resultsPanel->setGuessResultsVisible(false);
    resultsPanel->setShowFinalResults(false);

    // 重置後，如果 Level-Match 啟用，重新開始校準
    if (manager->isLevelMatchEnabled())
    {
        manager->resetLevelMatching();
        manager->startCalibration();
    }
}

void BlindCardEditor::onNextRoundClicked()
{
    auto phase = manager->getPhase();
    auto mode = manager->getRatingMode();

    // If in BlindTesting phase
    if (phase == blindcard::GamePhase::BlindTesting)
    {
        // Guess mode: reveal results with scoring and countdown
        if (mode == blindcard::RatingMode::Guess)
        {
            int currentRound = manager->getCurrentRound() + 1;
            int totalRounds = manager->getTotalRounds();

            resultsPanel->setRoundInfo(currentRound, totalRounds);
            buildGuessResults();
            resultsPanel->accumulateGuessResults();
            resultsPanel->setGuessResultsVisible(true);

            bool isLastRound = (currentRound >= totalRounds);
            resultsPanel->setShowFinalResults(isLastRound);

            // Start countdown timer (3 seconds) - only if not last round
            if (!isLastRound)
            {
                autoAdvanceStartTime = juce::Time::currentTimeMillis();
                autoAdvanceCountdown = kAutoAdvanceSeconds;
                resultsPanel->setCountdown(kAutoAdvanceSeconds);
            }

            // Reveal cards (only in Guess mode)
            manager->reveal();
            pokerTable->revealAllCards(100);
        }
        else if (mode == blindcard::RatingMode::Stars)
        {
            // Stars mode: just go to next round WITHOUT revealing
            // Cards stay face-down, ratings are preserved
            manager->nextRound();
        }
    }
    else if (phase == blindcard::GamePhase::Revealed)
    {
        // User clicked manually during reveal - cancel auto-advance and go to next round immediately
        autoAdvanceStartTime = 0;
        autoAdvanceCountdown = 0;
        resultsPanel->setCountdown(0);

        resultsPanel->setGuessResultsVisible(false);
        resultsPanel->setShowFinalResults(false);

        manager->nextRound();
    }
}

void BlindCardEditor::onRoundsChanged(int rounds)
{
    manager->setTotalRounds(rounds);

    // Q&A 模式時，Rounds 同時控制問題數量
    if (manager->getRatingMode() == blindcard::RatingMode::QA)
    {
        manager->setQAQuestionCount(rounds);
    }
}

void BlindCardEditor::onAutoGainChanged(bool enabled)
{
    manager->setLevelMatchEnabled(enabled);

    // 啟用時自動開始校準（Setup 階段）
    if (enabled && manager->getPhase() == blindcard::GamePhase::Setup)
        manager->startCalibration();
}

void BlindCardEditor::onQAQuestionsChanged(int questions)
{
    manager->setQAQuestionCount(questions);
}

//==============================================================================
// Event handlers - ResultsPanel

void BlindCardEditor::onSubmitGuesses()
{
    // In Guess mode, submit all guesses and show results
    int currentRound = manager->getCurrentRound() + 1;  // 1-based for display
    int totalRounds = manager->getTotalRounds();

    resultsPanel->setRoundInfo(currentRound, totalRounds);
    resultsPanel->setGuessResultsVisible(true);

    // Build results first so we can accumulate them
    buildGuessResults();
    resultsPanel->accumulateGuessResults();

    // Check if this is the last round
    bool isLastRound = (currentRound >= totalRounds);
    resultsPanel->setShowFinalResults(isLastRound);

    manager->reveal();
    pokerTable->revealAllCards(100);  // Staggered 100ms animation
}

//==============================================================================
// Helpers

std::vector<std::string> BlindCardEditor::getTrackNames() const
{
    std::vector<std::string> names;
    auto cards = manager->getCards();

    for (const auto& card : cards)
    {
        names.push_back(card.realTrackName.toStdString());
    }

    return names;
}

void BlindCardEditor::buildStarsResults()
{
    std::vector<StarsResult> results;
    auto cards = manager->getCards();
    int currentRound = manager->getCurrentRound();

    for (const auto& card : cards)
    {
        StarsResult result;
        result.trackName = card.realTrackName.toStdString();
        result.cardPosition = card.displayPosition;

        // Current round's rating (for BlindTesting display)
        if (currentRound >= 0 && currentRound < card.rounds.size())
            result.currentRating = card.rounds[currentRound].rating;
        else
            result.currentRating = 0;

        // Average rating across all rounds (for Revealed display)
        result.averageRating = card.getAverageRating();

        results.push_back(result);
    }

    resultsPanel->setStarsResults(results);
}

void BlindCardEditor::buildGuessResults()
{
    std::vector<GuessResult> results;
    auto cards = manager->getCards();
    int currentRound = manager->getCurrentRound();

    for (size_t i = 0; i < cards.size(); ++i)
    {
        const auto& card = cards[static_cast<int>(i)];
        GuessResult result;
        result.cardPosition = card.displayPosition;
        result.actualTrack = card.realTrackName.toStdString();

        if (currentRound < card.rounds.size())
        {
            int guessedId = card.rounds[currentRound].guessedTrackId;
            if (guessedId >= 0 && guessedId < cards.size())
            {
                result.guessedTrack = cards[guessedId].realTrackName.toStdString();
                result.isCorrect = (guessedId == card.id);
            }
        }

        results.push_back(result);
    }

    resultsPanel->setGuessResults(results);
}

void BlindCardEditor::buildQAResults()
{
    std::vector<QAResult> results;
    auto cards = manager->getCards();
    const auto& qaState = manager->getQAState();

    // Only show results for questions that have been answered
    // (answers.size() <= askedCardIds.size())
    int answeredCount = qaState.answers.size();

    for (int i = 0; i < answeredCount; ++i)
    {
        int cardId = qaState.askedCardIds[i];
        bool wasCorrect = qaState.answers[i];

        // Find card by ID
        for (const auto& card : cards)
        {
            if (card.id == cardId)
            {
                QAResult result;
                result.pluginName = card.realTrackName.toStdString();
                result.cardPosition = card.displayPosition;
                result.wasCorrect = wasCorrect;
                results.push_back(result);
                break;
            }
        }
    }

    resultsPanel->setQAResults(results);
}

//==============================================================================
// Developer Mode

bool BlindCardEditor::keyPressed(const juce::KeyPress& key, juce::Component*)
{
    // Spacebar to play/pause in Standalone mode
    if (isStandaloneMode && audioEngine && key.isKeyCode(juce::KeyPress::spaceKey))
    {
        audioEngine->togglePlayPause();
        return true;
    }

    // Press 'D' to toggle dev panel
    if (key.getTextCharacter() == 'd' || key.getTextCharacter() == 'D')
    {
        devPanelVisible = !devPanelVisible;
        if (devPanel)
            devPanel->setVisible(devPanelVisible);
        resized();
        repaint();
        return true;
    }

    // Card navigation helper lambda
    auto selectCardAtIndex = [this](int newIndex, const std::vector<std::pair<int, int>>& sortedCards, int cardCount)
    {
        if (newIndex >= 0 && newIndex < cardCount)
        {
            manager->selectCard(sortedCards[newIndex].second);

            // In Standalone mode, switch audio playback to selected card
            if (isStandaloneMode && audioEngine)
            {
                audioEngine->switchToCard(sortedCards[newIndex].first);
            }
            return true;
        }
        return false;
    };

    // Get card info for navigation
    auto cards = manager->getCards();
    int cardCount = static_cast<int>(cards.size());
    if (cardCount < 2)
        return false;

    // Build a sorted list of cards by displayPosition
    std::vector<std::pair<int, int>> sortedCards;  // (displayPosition, cardId)
    for (const auto& card : cards)
    {
        sortedCards.push_back({ card.displayPosition, card.id });
    }
    std::sort(sortedCards.begin(), sortedCards.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    // =========================================================================
    // Number keys 1-8: Direct card selection by display position
    // (Logic Pro compatible - number keys are not intercepted for transport)
    // =========================================================================
    char keyChar = static_cast<char>(key.getTextCharacter());
    if (keyChar >= '1' && keyChar <= '8')
    {
        int targetPosition = keyChar - '1';  // Convert '1'-'8' to 0-7
        if (targetPosition < cardCount)
        {
            return selectCardAtIndex(targetPosition, sortedCards, cardCount);
        }
        return true;  // Consumed even if position doesn't exist
    }

    // =========================================================================
    // Tab / Shift+Tab: Next / Previous card
    // (Logic Pro compatible alternative to arrow keys)
    // =========================================================================
    int currentSelectedId = manager->getSelectedCardId();
    int currentIndex = -1;

    // Find current selected card's index in sorted list
    for (int i = 0; i < static_cast<int>(sortedCards.size()); ++i)
    {
        if (sortedCards[i].second == currentSelectedId)
        {
            currentIndex = i;
            break;
        }
    }

    if (key.isKeyCode(juce::KeyPress::tabKey))
    {
        if (currentIndex < 0)
        {
            return selectCardAtIndex(0, sortedCards, cardCount);
        }

        int newIndex;
        if (key.getModifiers().isShiftDown())
        {
            // Shift+Tab: Previous card
            newIndex = (currentIndex - 1 + cardCount) % cardCount;
        }
        else
        {
            // Tab: Next card
            newIndex = (currentIndex + 1) % cardCount;
        }
        return selectCardAtIndex(newIndex, sortedCards, cardCount);
    }

    // =========================================================================
    // [ and ]: Previous / Next card
    // (Pro Tools style navigation, Logic Pro compatible)
    // =========================================================================
    if (keyChar == '[')
    {
        if (currentIndex < 0)
            return selectCardAtIndex(0, sortedCards, cardCount);
        int newIndex = (currentIndex - 1 + cardCount) % cardCount;
        return selectCardAtIndex(newIndex, sortedCards, cardCount);
    }
    else if (keyChar == ']')
    {
        if (currentIndex < 0)
            return selectCardAtIndex(0, sortedCards, cardCount);
        int newIndex = (currentIndex + 1) % cardCount;
        return selectCardAtIndex(newIndex, sortedCards, cardCount);
    }

    // =========================================================================
    // Arrow keys: Standard navigation (may be intercepted by Logic Pro)
    // =========================================================================
    // If no card selected, select first card
    if (currentIndex < 0)
    {
        if (key.isKeyCode(juce::KeyPress::leftKey) ||
            key.isKeyCode(juce::KeyPress::rightKey) ||
            key.isKeyCode(juce::KeyPress::upKey) ||
            key.isKeyCode(juce::KeyPress::downKey))
        {
            return selectCardAtIndex(0, sortedCards, cardCount);
        }
        return false;
    }

    int newIndex = currentIndex;

    // Calculate new index based on arrow key
    // Layout: cards are arranged in rows of 4
    int cardsPerRow = (cardCount <= 4) ? cardCount : 4;
    int currentCol = currentIndex % cardsPerRow;

    if (key.isKeyCode(juce::KeyPress::leftKey))
    {
        newIndex = (currentIndex - 1 + cardCount) % cardCount;
    }
    else if (key.isKeyCode(juce::KeyPress::rightKey))
    {
        newIndex = (currentIndex + 1) % cardCount;
    }
    else if (key.isKeyCode(juce::KeyPress::upKey))
    {
        // Move to row above (wrap around if at top)
        if (currentIndex >= cardsPerRow)
            newIndex = currentIndex - cardsPerRow;
        else if (cardCount > cardsPerRow)
        {
            // Wrap to bottom row
            int bottomRowStart = ((cardCount - 1) / cardsPerRow) * cardsPerRow;
            newIndex = juce::jmin(bottomRowStart + currentCol, cardCount - 1);
        }
    }
    else if (key.isKeyCode(juce::KeyPress::downKey))
    {
        // Move to row below (wrap around if at bottom)
        if (currentIndex + cardsPerRow < cardCount)
            newIndex = currentIndex + cardsPerRow;
        else if (cardCount > cardsPerRow)
        {
            // Wrap to top row
            newIndex = currentCol;
        }
    }
    else
    {
        return false;  // Not a navigation key
    }

    // Select the card at new index
    if (newIndex != currentIndex)
    {
        return selectCardAtIndex(newIndex, sortedCards, cardCount);
    }

    return true;
}

void BlindCardEditor::mouseDown(const juce::MouseEvent& event)
{
    // 點擊插件視窗任何位置時獲取鍵盤焦點
    // 這對於 Logic Pro 等 DAW 特別重要，因為 DAW 會攔截鍵盤事件
    if (!hasKeyboardFocus(true))
    {
        grabKeyboardFocus();
    }

    // 呼叫父類別處理
    juce::AudioProcessorEditor::mouseDown(event);
}

void BlindCardEditor::setupDevPanel()
{
    // Dev panel (hidden by default, toggle with 'D' key)
    devPanel = std::make_unique<juce::Component>();
    addAndMakeVisible(*devPanel);
    devPanel->setVisible(false);

    // Title label
    trackCountLabel = std::make_unique<juce::Label>();
    trackCountLabel->setText("Track Count:", juce::dontSendNotification);
    trackCountLabel->setFont(FontManager::getInstance().getBebasNeue(14.0f));
    trackCountLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    devPanel->addAndMakeVisible(*trackCountLabel);

    // Track count slider (2-8)
    trackCountSlider = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal,
                                                       juce::Slider::TextBoxRight);
    trackCountSlider->setRange(2.0, 8.0, 1.0);
    trackCountSlider->setValue(4.0);
    trackCountSlider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 24);
    trackCountSlider->setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    trackCountSlider->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xFF333333));
    devPanel->addAndMakeVisible(*trackCountSlider);

    // Apply button
    applyButton = std::make_unique<juce::TextButton>("Apply");
    applyButton->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF4CAF50));
    applyButton->setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    applyButton->onClick = [this]()
    {
        int count = static_cast<int>(trackCountSlider->getValue());
        applyTrackCount(count);
    };
    devPanel->addAndMakeVisible(*applyButton);
}

void BlindCardEditor::toggleDevPanel()
{
    devPanelVisible = !devPanelVisible;
    devPanel->setVisible(devPanelVisible);
    resized();
    repaint();
}

void BlindCardEditor::applyTrackCount(int count)
{
    // Set exact card count (clears and rebuilds)
    manager->setTestCardCount(count);

    // Resize window to fit cards (larger for 5+ cards)
    if (count > 4)
    {
        // Two rows need more height: ~900px minimum
        int newHeight = juce::jmax(850, getHeight());
        setSize(getWidth(), newHeight);
    }
    else
    {
        // Single row can use default height
        setSize(getWidth(), kMinHeight);
    }

    updateFromManager();
}

//==============================================================================
// Standalone Mode Implementation
//==============================================================================

void BlindCardEditor::setupStandaloneMode()
{
    // Create audio engine and initialize audio output
    audioEngine = std::make_unique<StandaloneAudioEngine>();
    audioEngine->initialize();  // Sets up audio device output

    // Create preset manager
    presetManager = std::make_unique<StandalonePresetManager>();

    // Setup preset UI (combo box and buttons)
    setupPresetUI();

    // Create transport bar
    transportBar = std::make_unique<TransportBar>();
    transportBar->onPlayPauseClicked = [this]() { onTransportPlayPause(); };
    transportBar->onSkipBackward = [this]() { audioEngine->skip(-StandaloneAudioEngine::kSkipSeconds); };
    transportBar->onSkipForward = [this]() { audioEngine->skip(StandaloneAudioEngine::kSkipSeconds); };
    transportBar->onSeek = [this](double pos) { onTransportSeek(pos); };
    transportBar->setEnabled(false);  // Disabled until audio is loaded
    addAndMakeVisible(*transportBar);

    // Create card count control
    cardCountControl = std::make_unique<CardCountControl>();
    cardCountControl->setCount(2);  // Default 2 cards in Standalone
    cardCountControl->onCountChanged = [this](int count) { onCardCountChanged(count); };
    addAndMakeVisible(*cardCountControl);

    // Enable standalone mode on manager (prevents auto track names)
    manager->setStandaloneMode(true);

    // Set initial card count (Standalone defaults to 2 cards)
    manager->setTestCardCount(2);

    // Enable standalone mode on PokerTable, HeaderBar, and ControlPanel
    pokerTable->setStandaloneMode(true);
    headerBar->setStandaloneMode(true);  // Hides "Now Playing" to make room for TransportBar
    controlPanel->setStandaloneMode(true);  // Hides Level-Match (doesn't work without DAW)

    // Connect file drop callbacks for each card
    int cardCount = static_cast<int>(manager->getCards().size());
    for (int i = 0; i < cardCount; ++i)
    {
        auto* card = pokerTable->getCard(i);
        if (card)
        {
            card->onFileDropped = [this, i](const juce::File& file) { onCardFileDropped(i, file); };
            card->onRemoveFileClicked = [this, i]() { onCardRemoveFile(i); };
        }
    }

    // Increase window height to accommodate preset row (36px)
    setSize(getWidth(), kMinHeight + 36);

    // Trigger layout update to position the new components
    resized();

    // Ensure components are on top of HeaderBar
    transportBar->toFront(false);
    cardCountControl->toFront(false);
}

void BlindCardEditor::onTransportPlayPause()
{
    if (!audioEngine) return;

    audioEngine->togglePlayPause();
    transportBar->setPlaying(audioEngine->isPlaying());
}

void BlindCardEditor::onTransportSeek(double normalizedPosition)
{
    if (!audioEngine) return;

    double totalLength = audioEngine->getTotalLengthSeconds();
    double newPosition = normalizedPosition * totalLength;
    audioEngine->seek(newPosition);
}

void BlindCardEditor::onCardCountChanged(int newCount)
{
    // Update card count in manager
    manager->setTestCardCount(newCount);

    // Resize window if needed (add 36px for preset row in standalone mode)
    int standaloneExtra = isStandaloneMode ? 36 : 0;
    if (newCount > 4)
    {
        int newHeight = juce::jmax(850 + standaloneExtra, getHeight());
        setSize(getWidth(), newHeight);
    }
    else
    {
        setSize(getWidth(), kMinHeight + standaloneExtra);
    }

    // Update PokerTable with new cards FIRST (creates the card components)
    updateFromManager();

    // THEN re-enable standalone mode and reconnect callbacks for ALL cards
    pokerTable->setStandaloneMode(true);
    for (int i = 0; i < newCount; ++i)
    {
        auto* card = pokerTable->getCard(i);
        if (card)
        {
            card->setStandaloneMode(true);  // Ensure each card has standalone mode enabled
            card->onFileDropped = [this, i](const juce::File& file) { onCardFileDropped(i, file); };
            card->onRemoveFileClicked = [this, i]() { onCardRemoveFile(i); };
        }
    }
}

void BlindCardEditor::onCardFileDropped(int cardIndex, const juce::File& file)
{
    if (!audioEngine) return;

    // Load file into audio engine
    bool success = audioEngine->loadFile(cardIndex, file);

    if (success)
    {
        // Update card UI with file info
        auto* card = pokerTable->getCard(cardIndex);
        if (card)
        {
            const auto* slot = audioEngine->getSlot(cardIndex);
            if (slot)
            {
                card->setLoadedAudioFile(file, slot->lengthInSeconds);
            }
        }

        // Update BlindCardManager with track name (for ResultsPanel display)
        juce::String trackName = file.getFileNameWithoutExtension();
        manager->setTrackName(cardIndex, trackName);

        // Enable transport if we have audio
        transportBar->setEnabled(audioEngine->hasAnyAudioLoaded());
        transportBar->setDuration(audioEngine->getTotalLengthSeconds());

        // Auto-switch to this card and start playback
        audioEngine->switchToCard(cardIndex);
        pokerTable->setSelectedCard(cardIndex);
    }
    else
    {
        // Show error (could add toast notification here)
        DBG("Failed to load audio file: " + file.getFullPathName());
    }
}

void BlindCardEditor::onCardRemoveFile(int cardIndex)
{
    if (!audioEngine) return;

    // Check if we're removing the currently active card
    bool wasActiveCard = (audioEngine->getActiveCardId() == cardIndex);

    // Unload from engine
    audioEngine->unloadFile(cardIndex);

    // Update card UI
    auto* card = pokerTable->getCard(cardIndex);
    if (card)
    {
        card->setLoadedAudioFile(juce::File(), 0.0);
    }

    // Clear track name in BlindCardManager
    manager->setTrackName(cardIndex, "");

    // If we removed the active card, switch to another card with audio
    if (wasActiveCard && audioEngine->hasAnyAudioLoaded())
    {
        // Find first card with audio loaded
        for (int i = 0; i < StandaloneAudioEngine::kMaxSlots; ++i)
        {
            const auto* slot = audioEngine->getSlot(i);
            if (slot && slot->isLoaded)
            {
                audioEngine->switchToCard(i);
                pokerTable->setSelectedCard(i);
                break;
            }
        }
    }

    // Update transport state
    transportBar->setEnabled(audioEngine->hasAnyAudioLoaded());
    transportBar->setDuration(audioEngine->getTotalLengthSeconds());
}

void BlindCardEditor::updateTransportFromEngine()
{
    if (!audioEngine || !transportBar) return;

    transportBar->setPosition(audioEngine->getCurrentPositionSeconds());
    transportBar->setPlaying(audioEngine->isPlaying());

    // Update Now Playing display
    updateNowPlayingDisplay();
}

void BlindCardEditor::updateNowPlayingDisplay()
{
    if (!audioEngine || !nowPlayingTrackName) return;

    int activeCardId = audioEngine->getActiveCardId();
    const auto* slot = audioEngine->getSlot(activeCardId);

    if (slot && slot->isLoaded)
    {
        // Get track name from file name (without extension)
        juce::String trackName = slot->file.getFileNameWithoutExtension();

        // Get RMS dB level
        float rmsDb = audioEngine->getCurrentRMSdB();

        // Format the display text with track name and dB level
        juce::String displayText;
        if (audioEngine->isPlaying() && rmsDb > -100.0f)
        {
            // Show dB level when playing
            displayText = trackName + "  " + juce::String(rmsDb, 1) + " dB";
        }
        else
        {
            displayText = trackName;
        }

        nowPlayingTrackName->setText(displayText, juce::dontSendNotification);
        nowPlayingTrackName->setColour(juce::Label::textColourId, juce::Colours::white);
    }
    else
    {
        nowPlayingTrackName->setText("No audio loaded", juce::dontSendNotification);
        nowPlayingTrackName->setColour(juce::Label::textColourId, juce::Colour(0xFFAAAAAA));  // Light gray
    }
}

//==============================================================================
// Preset System Implementation
//==============================================================================

void BlindCardEditor::setupPresetUI()
{
    // Create custom LookAndFeel with proper theme
    presetUILookAndFeel = std::make_unique<PresetUILookAndFeel>();
    presetUILookAndFeel->updateColours(ThemeManager::getInstance().isDark());

    // Preset combo box
    presetComboBox = std::make_unique<juce::ComboBox>();
    presetComboBox->setTextWhenNothingSelected("Select Preset...");
    presetComboBox->setEditableText(true);  // Allow typing new preset names
    presetComboBox->setLookAndFeel(presetUILookAndFeel.get());
    presetComboBox->onChange = [this]()
    {
        int selectedIndex = presetComboBox->getSelectedItemIndex();
        if (selectedIndex >= 0)
        {
            onLoadPreset(selectedIndex);
        }
    };
    addAndMakeVisible(*presetComboBox);

    // Save button
    savePresetButton = std::make_unique<juce::TextButton>("SAVE");
    savePresetButton->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF4CAF50));
    savePresetButton->setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    savePresetButton->setLookAndFeel(presetUILookAndFeel.get());
    savePresetButton->onClick = [this]() { onSavePreset(); };
    addAndMakeVisible(*savePresetButton);

    // Delete button
    deletePresetButton = std::make_unique<juce::TextButton>("DELETE");
    deletePresetButton->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFFE53935));
    deletePresetButton->setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    deletePresetButton->setLookAndFeel(presetUILookAndFeel.get());
    deletePresetButton->onClick = [this]() { onDeletePreset(); };
    addAndMakeVisible(*deletePresetButton);

    // Import Files button (for batch import)
    importFilesButton = std::make_unique<juce::TextButton>("IMPORT FILES");
    importFilesButton->setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2196F3));  // Blue
    importFilesButton->setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    importFilesButton->setLookAndFeel(presetUILookAndFeel.get());
    importFilesButton->onClick = [this]() { onImportFiles(); };
    addAndMakeVisible(*importFilesButton);

    // Now Playing label (right side of preset row)
    nowPlayingLabel = std::make_unique<juce::Label>();
    nowPlayingLabel->setText("Now Playing", juce::dontSendNotification);
    nowPlayingLabel->setColour(juce::Label::textColourId, juce::Colour(0xFFD4AF37));  // Gold
    nowPlayingLabel->setFont(FontManager::getInstance().getMedium(15.0f));
    nowPlayingLabel->setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(*nowPlayingLabel);

    // Now Playing track name
    nowPlayingTrackName = std::make_unique<juce::Label>();
    nowPlayingTrackName->setText("No audio loaded", juce::dontSendNotification);
    nowPlayingTrackName->setColour(juce::Label::textColourId, juce::Colours::white);
    nowPlayingTrackName->setFont(FontManager::getInstance().getMedium(16.0f));
    nowPlayingTrackName->setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(*nowPlayingTrackName);

    // Populate preset list
    refreshPresetList();
}

void BlindCardEditor::onSavePreset()
{
    if (!presetManager) return;

    // Get preset name from combo box text
    juce::String presetName = presetComboBox->getText().trim();

    if (presetName.isEmpty())
    {
        // Generate default name with timestamp
        presetName = "Preset " + juce::Time::getCurrentTime().toString(true, true, false, false);
    }

    // Confirm overwrite if preset already exists
    if (presetManager->presetExists(presetName))
    {
        auto options = juce::MessageBoxOptions()
            .withIconType(juce::MessageBoxIconType::QuestionIcon)
            .withTitle("Overwrite Preset")
            .withMessage("Preset \"" + presetName + "\" already exists. Overwrite?")
            .withButton("Overwrite")
            .withButton("Cancel")
            .withAssociatedComponent(this);

        juce::AlertWindow::showAsync(options, [this, presetName](int result)
        {
            if (result != 1)  // 1 = first button (Overwrite)
                return;

            // Get current configuration and save
            auto data = getCurrentPresetData();
            data.name = presetName;

            if (presetManager->savePreset(presetName, data))
            {
                DBG("Saved preset: " + presetName);
                refreshPresetList();

                // Select the newly saved preset
                auto names = presetManager->getPresetNames();
                for (int i = 0; i < names.size(); ++i)
                {
                    if (names[i] == presetName)
                    {
                        presetComboBox->setSelectedItemIndex(i, juce::dontSendNotification);
                        break;
                    }
                }
            }
            else
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    "Save Failed",
                    "Failed to save preset \"" + presetName + "\".");
            }
        });
        return;
    }

    // Get current configuration and save (no overwrite needed)
    auto data = getCurrentPresetData();
    data.name = presetName;

    if (presetManager->savePreset(presetName, data))
    {
        DBG("Saved preset: " + presetName);
        refreshPresetList();

        // Select the newly saved preset
        auto names = presetManager->getPresetNames();
        for (int i = 0; i < names.size(); ++i)
        {
            if (names[i] == presetName)
            {
                presetComboBox->setSelectedItemIndex(i, juce::dontSendNotification);
                break;
            }
        }
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Save Failed",
            "Failed to save preset \"" + presetName + "\".");
    }
}

void BlindCardEditor::onLoadPreset(int presetIndex)
{
    if (!presetManager) return;

    auto names = presetManager->getPresetNames();
    if (presetIndex < 0 || presetIndex >= names.size())
        return;

    juce::String presetName = names[presetIndex];
    StandalonePresetManager::PresetData data;

    if (presetManager->loadPreset(presetName, data))
    {
        DBG("Loading preset: " + presetName);
        applyPresetData(data);
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Load Failed",
            "Failed to load preset \"" + presetName + "\".");
    }
}

void BlindCardEditor::onDeletePreset()
{
    if (!presetManager) return;

    int selectedIndex = presetComboBox->getSelectedItemIndex();
    if (selectedIndex < 0)
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            "No Preset Selected",
            "Please select a preset to delete.");
        return;
    }

    auto names = presetManager->getPresetNames();
    if (selectedIndex >= names.size())
        return;

    juce::String presetName = names[selectedIndex];

    // Confirm deletion
    auto options = juce::MessageBoxOptions()
        .withIconType(juce::MessageBoxIconType::QuestionIcon)
        .withTitle("Delete Preset")
        .withMessage("Are you sure you want to delete \"" + presetName + "\"?")
        .withButton("Delete")
        .withButton("Cancel")
        .withAssociatedComponent(this);

    juce::AlertWindow::showAsync(options, [this, presetName](int result)
    {
        if (result != 1)  // 1 = first button (Delete)
            return;

        if (presetManager->deletePreset(presetName))
        {
            DBG("Deleted preset: " + presetName);
            refreshPresetList();
            presetComboBox->setSelectedItemIndex(-1);
        }
        else
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "Delete Failed",
                "Failed to delete preset \"" + presetName + "\".");
        }
    });
}

void BlindCardEditor::refreshPresetList()
{
    if (!presetComboBox || !presetManager) return;

    // Remember current selection
    juce::String currentText = presetComboBox->getText();

    presetComboBox->clear(juce::dontSendNotification);

    auto names = presetManager->getPresetNames();
    int itemId = 1;
    int selectedIndex = -1;

    for (const auto& name : names)
    {
        presetComboBox->addItem(name, itemId);
        if (name == currentText)
            selectedIndex = itemId - 1;
        itemId++;
    }

    // Restore selection if found
    if (selectedIndex >= 0)
        presetComboBox->setSelectedItemIndex(selectedIndex, juce::dontSendNotification);
}

StandalonePresetManager::PresetData BlindCardEditor::getCurrentPresetData() const
{
    StandalonePresetManager::PresetData data;

    // Get current card count
    auto cards = manager->getCards();
    data.cardCount = static_cast<int>(cards.size());

    // Collect slot data from audio engine
    if (audioEngine)
    {
        for (int i = 0; i < data.cardCount && i < StandaloneAudioEngine::kMaxSlots; ++i)
        {
            const auto* slot = audioEngine->getSlot(i);
            StandalonePresetManager::SlotData slotData;
            slotData.cardIndex = i;

            if (slot && slot->isLoaded)
            {
                slotData.filePath = slot->file.getFullPathName();
                slotData.hasFile = true;
            }
            else
            {
                slotData.hasFile = false;
            }

            data.slots.push_back(slotData);
        }
    }

    return data;
}

void BlindCardEditor::applyPresetData(const StandalonePresetManager::PresetData& data)
{
    if (!audioEngine) return;

    // First, set the card count
    if (data.cardCount >= blindcard::GameState::MinCards &&
        data.cardCount <= blindcard::GameState::MaxCards)
    {
        if (cardCountControl)
            cardCountControl->setCount(data.cardCount);
        onCardCountChanged(data.cardCount);
    }

    // Clear all existing audio
    for (int i = 0; i < StandaloneAudioEngine::kMaxSlots; ++i)
    {
        audioEngine->unloadFile(i);
        auto* card = pokerTable->getCard(i);
        if (card)
            card->setLoadedAudioFile(juce::File(), 0.0);
    }

    // Load audio files from preset
    bool anyLoaded = false;
    int firstLoadedCard = -1;

    for (const auto& slot : data.slots)
    {
        if (slot.hasFile && slot.cardIndex >= 0 && slot.cardIndex < data.cardCount)
        {
            juce::File file(slot.filePath);
            if (file.existsAsFile())
            {
                bool success = audioEngine->loadFile(slot.cardIndex, file);
                if (success)
                {
                    auto* card = pokerTable->getCard(slot.cardIndex);
                    if (card)
                    {
                        const auto* engineSlot = audioEngine->getSlot(slot.cardIndex);
                        if (engineSlot)
                        {
                            card->setLoadedAudioFile(file, engineSlot->lengthInSeconds);
                        }
                    }
                    anyLoaded = true;
                    if (firstLoadedCard < 0)
                        firstLoadedCard = slot.cardIndex;
                }
            }
            else
            {
                DBG("Preset file not found: " + slot.filePath);
            }
        }
    }

    // Update transport state
    transportBar->setEnabled(anyLoaded);
    if (anyLoaded)
    {
        transportBar->setDuration(audioEngine->getTotalLengthSeconds());

        // Select first loaded card
        if (firstLoadedCard >= 0)
        {
            audioEngine->switchToCard(firstLoadedCard);
            pokerTable->setSelectedCard(firstLoadedCard);
        }
    }
}

//==============================================================================
// Batch Import Implementation
//==============================================================================

void BlindCardEditor::onImportFiles()
{
    if (!audioEngine) return;

    // Create file chooser with multi-selection enabled
    fileChooser = std::make_unique<juce::FileChooser>(
        "Select Audio Files to Import",
        juce::File::getSpecialLocation(juce::File::userMusicDirectory),
        "*.wav;*.mp3;*.aiff;*.aif",
        true  // Use native dialog
    );

    // Launch async file chooser with multiple file selection
    auto chooserFlags = juce::FileBrowserComponent::openMode
                      | juce::FileBrowserComponent::canSelectFiles
                      | juce::FileBrowserComponent::canSelectMultipleItems;

    fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& chooser)
    {
        auto results = chooser.getResults();

        if (results.isEmpty())
            return;

        // Sort files by name for predictable ordering
        std::vector<juce::File> sortedFiles;
        for (const auto& file : results)
            sortedFiles.push_back(file);

        std::sort(sortedFiles.begin(), sortedFiles.end(),
            [](const juce::File& a, const juce::File& b)
            {
                return a.getFileName().compareNatural(b.getFileName()) < 0;
            });

        int numFiles = static_cast<int>(sortedFiles.size());
        int maxCards = StandaloneAudioEngine::kMaxSlots;

        // If importing more files than current card count, expand to fit
        int currentCardCount = static_cast<int>(manager->getCards().size());
        int targetCardCount = std::min(numFiles, maxCards);

        if (targetCardCount > currentCardCount)
        {
            // Use onCardCountChanged to properly update UI and callbacks
            onCardCountChanged(targetCardCount);
            if (cardCountControl)
                cardCountControl->setCount(targetCardCount);
            currentCardCount = targetCardCount;
        }

        // Load files into consecutive card slots
        int filesLoaded = 0;
        int firstLoadedCard = -1;

        for (int i = 0; i < std::min(numFiles, currentCardCount); ++i)
        {
            const auto& file = sortedFiles[i];

            // Check if format is supported
            if (!audioEngine->isFormatSupported(file))
            {
                DBG("Skipping unsupported file: " + file.getFileName());
                continue;
            }

            // Load file into slot
            bool success = audioEngine->loadFile(i, file);
            if (success)
            {
                // Update card UI
                auto* card = pokerTable->getCard(i);
                if (card)
                {
                    const auto* slot = audioEngine->getSlot(i);
                    if (slot)
                    {
                        card->setLoadedAudioFile(file, slot->lengthInSeconds);
                    }
                }

                filesLoaded++;
                if (firstLoadedCard < 0)
                    firstLoadedCard = i;

                DBG("Loaded file " + juce::String(i + 1) + ": " + file.getFileName());
            }
        }

        // Update transport bar
        if (filesLoaded > 0)
        {
            transportBar->setEnabled(true);
            transportBar->setDuration(audioEngine->getTotalLengthSeconds());

            // Select first loaded card
            if (firstLoadedCard >= 0)
            {
                audioEngine->switchToCard(firstLoadedCard);
                pokerTable->setSelectedCard(firstLoadedCard);
            }

            // Update now playing display
            updateNowPlayingDisplay();
        }

        // Show summary message
        juce::String message;
        if (filesLoaded == numFiles)
        {
            message = "Successfully imported " + juce::String(filesLoaded) + " file(s).";
        }
        else if (filesLoaded > 0)
        {
            message = "Imported " + juce::String(filesLoaded) + " of " + juce::String(numFiles)
                    + " files.\nSome files may have been skipped.";
        }
        else
        {
            message = "No files were imported.\nPlease check file formats (WAV, MP3, AIFF).";
        }

        // Show brief toast-style notification
        DBG(message);
    });
}

} // namespace BlindCard
