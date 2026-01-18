/*
  ==============================================================================

    BlindCardEditor.cpp
    Created: 2026-01-19
    Author:  BlindCard

  ==============================================================================
*/

#include "BlindCardEditor.h"
#include "../PluginProcessor.h"

namespace BlindCard
{

//==============================================================================
BlindCardEditor::BlindCardEditor(BlindCardProcessor& processor)
    : AudioProcessorEditor(processor)
    , processorRef(processor)
{
    // Set window constraints
    setResizable(true, true);
    setResizeLimits(kMinWidth, kMinHeight, kMaxWidth, kMaxHeight);
    setSize(kMinWidth, kMinHeight);

    // Subscribe to manager changes
    manager->addChangeListener(this);

    // Create UI components
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
    addAndMakeVisible(*controlPanel);

    resultsPanel = std::make_unique<ResultsPanel>();
    resultsPanel->onSubmitGuesses = [this]() { onSubmitGuesses(); };
    addAndMakeVisible(*resultsPanel);

    // Initial state sync
    updateFromManager();

    // Start update timer (for connection status, etc.)
    startTimerHz(30);
}

BlindCardEditor::~BlindCardEditor()
{
    stopTimer();
    manager->removeChangeListener(this);
}

//==============================================================================
void BlindCardEditor::paint(juce::Graphics& g)
{
    auto& tm = ThemeManager::getInstance();
    g.fillAll(tm.getColour(ColourId::Background));
}

void BlindCardEditor::resized()
{
    auto bounds = getLocalBounds();

    // Header at top
    headerBar->setBounds(bounds.removeFromTop(kHeaderHeight));

    // Mode selector below header
    auto modeSelectorArea = bounds.removeFromTop(kModeSelectorHeight);
    auto selectorBounds = modeSelectorArea.withSizeKeepingCentre(280, 40);
    modeSelector->setBounds(selectorBounds);

    // Question banner (Q&A mode) - only show if in Q&A mode
    auto currentMode = manager->getRatingMode();
    if (currentMode == blindcard::RatingMode::QA)
    {
        auto bannerArea = bounds.removeFromTop(kQuestionBannerHeight);
        questionBanner->setBounds(bannerArea.reduced(100, 4));
        questionBanner->setVisible(true);
    }
    else
    {
        questionBanner->setVisible(false);
    }

    // Main content area
    auto mainArea = bounds.reduced(16, 8);

    // Side panel on right
    auto sidePanelArea = mainArea.removeFromRight(kSidePanelWidth);
    mainArea.removeFromRight(16);  // Gap

    // Control panel at top of side panel
    auto controlPanelHeight = juce::jmin(sidePanelArea.getHeight() / 2, 350);
    controlPanel->setBounds(sidePanelArea.removeFromTop(controlPanelHeight));

    sidePanelArea.removeFromTop(12);  // Gap

    // Results panel fills remaining side panel space
    resultsPanel->setBounds(sidePanelArea);

    // Poker table fills remaining main area
    pokerTable->setBounds(mainArea);
}

//==============================================================================
void BlindCardEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    // Called when BlindCardManager state changes
    juce::MessageManager::callAsync([this]()
    {
        updateFromManager();
    });
}

void BlindCardEditor::timerCallback()
{
    // Update connection status
    int registeredCount = manager->getRegisteredCount();
    headerBar->setConnected(registeredCount > 0);
}

//==============================================================================
void BlindCardEditor::updateFromManager()
{
    // Sync card count
    auto cards = manager->getCards();
    int cardCount = cards.size();

    if (cardCount >= 2)
    {
        pokerTable->setCardCount(cardCount);
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

    // Sync phase
    auto phase = manager->getPhase();
    pokerTable->setPhase(phase);

    // Update card states
    updateCardStates();

    // Update mode-specific UI
    updateModeUI();
    updatePhaseUI();
    updateResultsPanel();

    // Trigger layout update for Q&A banner visibility
    resized();
    repaint();
}

void BlindCardEditor::updateCardStates()
{
    auto cards = manager->getCards();
    auto selectedId = manager->getSelectedCardId();

    for (size_t i = 0; i < cards.size(); ++i)
    {
        auto* card = pokerTable->getCard(static_cast<int>(i));
        if (card == nullptr)
            continue;

        const auto& slot = cards[static_cast<int>(i)];

        CardData data;
        data.position = slot.displayPosition;
        data.trackName = slot.realTrackName.toStdString();
        data.isRevealed = (manager->getPhase() == blindcard::GamePhase::Revealed);
        data.isSelected = (slot.id == selectedId);
        data.isPlaying = (slot.id == selectedId);

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
            controlPanel->setShuffleEnabled(manager->getRegisteredCount() >= 2);
            controlPanel->setRevealEnabled(false);
            controlPanel->setResetEnabled(false);
            controlPanel->setNextRoundEnabled(false);
            modeSelector->setLocked(false);
            break;

        case blindcard::GamePhase::BlindTesting:
            controlPanel->setShuffleEnabled(false);
            controlPanel->setRevealEnabled(true);
            controlPanel->setResetEnabled(true);
            controlPanel->setNextRoundEnabled(false);
            modeSelector->setLocked(true);
            break;

        case blindcard::GamePhase::Revealed:
            controlPanel->setShuffleEnabled(false);
            controlPanel->setRevealEnabled(false);
            controlPanel->setResetEnabled(true);
            controlPanel->setNextRoundEnabled(manager->getCurrentRound() < manager->getTotalRounds() - 1);
            modeSelector->setLocked(true);
            break;
    }
}

void BlindCardEditor::updateQAUI()
{
    const auto& qaState = manager->getQAState();

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
}

void BlindCardEditor::updateResultsPanel()
{
    auto mode = manager->getRatingMode();

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
    // TODO: Show settings dialog
}

void BlindCardEditor::onInfoClicked()
{
    // TODO: Show info/about dialog
}

void BlindCardEditor::onThemeToggled()
{
    // Theme is toggled in HeaderBar via ThemeManager
    repaint();
}

//==============================================================================
// Event handlers - ModeSelector

void BlindCardEditor::onModeChanged(blindcard::RatingMode mode)
{
    manager->setRatingMode(mode);
}

//==============================================================================
// Event handlers - PokerTable

void BlindCardEditor::onCardClicked(int cardIndex)
{
    auto cards = manager->getCards();
    if (cardIndex >= 0 && cardIndex < cards.size())
    {
        manager->selectCard(cards[cardIndex].id);
    }
}

void BlindCardEditor::onCardRatingChanged(int cardIndex, int rating)
{
    auto cards = manager->getCards();
    if (cardIndex >= 0 && cardIndex < cards.size())
    {
        manager->rateCard(cards[cardIndex].id, rating);
    }
}

void BlindCardEditor::onCardGuessChanged(int cardIndex, int trackIndex)
{
    auto cards = manager->getCards();
    if (cardIndex >= 0 && cardIndex < cards.size())
    {
        manager->setGuess(cards[cardIndex].id, trackIndex);
    }
}

void BlindCardEditor::onCardQASelected(int cardIndex)
{
    auto cards = manager->getCards();
    if (cardIndex >= 0 && cardIndex < cards.size())
    {
        manager->submitQAAnswer(cards[cardIndex].id);
    }
}

//==============================================================================
// Event handlers - ControlPanel

void BlindCardEditor::onShuffleClicked()
{
    manager->shuffle();
}

void BlindCardEditor::onRevealClicked()
{
    manager->reveal();
    pokerTable->revealAllCards(100);  // Staggered 100ms animation
}

void BlindCardEditor::onResetClicked()
{
    manager->reset();
}

void BlindCardEditor::onNextRoundClicked()
{
    manager->nextRound();
}

void BlindCardEditor::onRoundsChanged(int rounds)
{
    manager->setTotalRounds(rounds);
}

void BlindCardEditor::onAutoGainChanged(bool enabled)
{
    // TODO: Connect to level matching system
}

//==============================================================================
// Event handlers - ResultsPanel

void BlindCardEditor::onSubmitGuesses()
{
    // In Guess mode, submit all guesses and show results
    resultsPanel->setGuessResultsVisible(true);
    manager->reveal();
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

        if (currentRound < card.rounds.size())
        {
            result.rating = card.rounds[currentRound].rating;
        }

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

    for (int i = 0; i < qaState.askedCardIds.size(); ++i)
    {
        int cardId = qaState.askedCardIds[i];
        bool wasCorrect = i < qaState.answers.size() ? qaState.answers[i] : false;

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

} // namespace BlindCard
