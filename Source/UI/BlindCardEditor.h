/*
  ==============================================================================

    BlindCardEditor.h
    Created: 2026-01-19
    Author:  BlindCard

    Main editor component for the BlindCard plugin.
    Composes all UI components and connects to BlindCardManager.

    Window size: 900×600 (min) to 1400×900 (max)

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Theme/ThemeManager.h"
#include "Components/HeaderBar.h"
#include "Components/ModeSelector.h"
#include "Components/QuestionBanner.h"
#include "Components/PokerTable.h"
#include "Components/ControlPanel.h"
#include "Components/ResultsPanel.h"
#include "../Core/BlindCardManager.h"

// Forward declaration
class BlindCardProcessor;

namespace BlindCard
{

//==============================================================================
/**
 * BlindCardEditor is the main UI component for the BlindCard plugin.
 *
 * Layout (900×600 to 1400×900):
 * +----------------------------------------------------------+
 * | HeaderBar (56px)                                          |
 * +----------------------------------------------------------+
 * |           ModeSelector (centered, 40px)                   |
 * +----------------------------------------------------------+
 * |           QuestionBanner (Q&A mode only, 48px)            |
 * +----------------------------------------------------------+
 * |                                      |                    |
 * |                                      | ControlPanel       |
 * |           PokerTable                 | (320px width)      |
 * |           (expandable)               |                    |
 * |                                      |--------------------+
 * |                                      | ResultsPanel       |
 * |                                      | (280px width)      |
 * +----------------------------------------------------------+
 *
 * The editor connects to BlindCardManager for:
 * - Game state (phase, round, cards)
 * - Actions (shuffle, reveal, reset, next)
 * - Rating/guess data
 *
 * Theme switching is handled via ThemeManager singleton.
 */
class BlindCardEditor : public juce::AudioProcessorEditor,
                        public juce::ChangeListener,
                        public juce::Timer
{
public:
    //==========================================================================
    /**
     * Constructor.
     * @param processor The BlindCardProcessor instance
     */
    explicit BlindCardEditor(BlindCardProcessor& processor);

    /** Destructor */
    ~BlindCardEditor() override;

    //==========================================================================
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;

    // ChangeListener override (for BlindCardManager)
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    // Timer callback for periodic updates
    void timerCallback() override;

    //==========================================================================
    /** Window dimensions */
    static constexpr int kMinWidth = 900;
    static constexpr int kMinHeight = 600;
    static constexpr int kMaxWidth = 1400;
    static constexpr int kMaxHeight = 900;

    // Layout constants
    static constexpr int kHeaderHeight = 56;
    static constexpr int kModeSelectorHeight = 48;
    static constexpr int kQuestionBannerHeight = 56;
    static constexpr int kSidePanelWidth = 320;

private:
    //==========================================================================
    BlindCardProcessor& processorRef;
    blindcard::SharedBlindCardManager manager;

    //==========================================================================
    // UI Components
    std::unique_ptr<HeaderBar> headerBar;
    std::unique_ptr<ModeSelector> modeSelector;
    std::unique_ptr<QuestionBanner> questionBanner;
    std::unique_ptr<PokerTable> pokerTable;
    std::unique_ptr<ControlPanel> controlPanel;
    std::unique_ptr<ResultsPanel> resultsPanel;

    //==========================================================================
    // State sync
    void updateFromManager();
    void updateCardStates();
    void updateModeUI();
    void updatePhaseUI();
    void updateQAUI();
    void updateResultsPanel();

    //==========================================================================
    // Event handlers - HeaderBar
    void onSettingsClicked();
    void onInfoClicked();
    void onThemeToggled();

    // Event handlers - ModeSelector
    void onModeChanged(blindcard::RatingMode mode);

    // Event handlers - PokerTable
    void onCardClicked(int cardIndex);
    void onCardRatingChanged(int cardIndex, int rating);
    void onCardGuessChanged(int cardIndex, int trackIndex);
    void onCardQASelected(int cardIndex);

    // Event handlers - ControlPanel
    void onShuffleClicked();
    void onRevealClicked();
    void onResetClicked();
    void onNextRoundClicked();
    void onRoundsChanged(int rounds);
    void onAutoGainChanged(bool enabled);

    // Event handlers - ResultsPanel
    void onSubmitGuesses();

    //==========================================================================
    // Helpers
    std::vector<std::string> getTrackNames() const;
    void buildStarsResults();
    void buildGuessResults();
    void buildQAResults();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BlindCardEditor)
};

} // namespace BlindCard
