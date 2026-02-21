/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

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
#include "Components/QuickStartGuide.h"
#include "Components/SettingsPanel.h"
#include "Components/TransportBar.h"
#include "Components/CardCountControl.h"
#include "../Core/BlindCardManager.h"
#include "../Core/UpdateChecker.h"
#include "../Standalone/StandaloneAudioEngine.h"
#include "../Standalone/StandalonePresetManager.h"

// Forward declaration
class BlindCardProcessor;

namespace BlindCard
{

//==============================================================================
// Custom LookAndFeel for Preset UI
// - ComboBox: Noto Sans TC Medium (readable for longer text)
// - Buttons: Bebas Neue (display font for short labels)
// - Theme-aware: Supports light/dark mode
//==============================================================================
class PresetUILookAndFeel : public juce::LookAndFeel_V4
{
public:
    PresetUILookAndFeel();

    /** Update colors for light/dark theme */
    void updateColours(bool isDark);

    juce::Font getComboBoxFont(juce::ComboBox&) override;
    juce::Font getTextButtonFont(juce::TextButton&, int) override;
    juce::Font getPopupMenuFont() override;
};

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
                        public juce::Timer,
                        public juce::KeyListener
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

    // KeyListener override for dev mode
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;

    // Mouse handling for keyboard focus
    void mouseDown(const juce::MouseEvent& event) override;

    //==========================================================================
    /** Window dimensions */
    static constexpr int kMinWidth = 900;
    static constexpr int kMinHeight = 750;  // Taller default for better card spacing
    static constexpr int kMaxWidth = 1400;
    static constexpr int kMaxHeight = 900;

    // Layout constants
    static constexpr int kHeaderHeight = 72;
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
    std::unique_ptr<QuickStartGuide> quickStartGuide;
    std::unique_ptr<SettingsPanel> settingsPanel;

    //==========================================================================
    // Standalone mode components
    bool isStandaloneMode = false;
    std::unique_ptr<StandaloneAudioEngine> audioEngine;
    std::unique_ptr<TransportBar> transportBar;
    std::unique_ptr<CardCountControl> cardCountControl;

    // Preset management
    std::unique_ptr<StandalonePresetManager> presetManager;
    std::unique_ptr<juce::ComboBox> presetComboBox;
    std::unique_ptr<juce::TextButton> savePresetButton;
    std::unique_ptr<juce::TextButton> deletePresetButton;
    std::unique_ptr<juce::TextButton> importFilesButton;
    std::unique_ptr<juce::FileChooser> fileChooser;  // For async file dialog
    std::unique_ptr<PresetUILookAndFeel> presetUILookAndFeel;  // Theme-aware preset UI style

    // Now Playing display (standalone mode)
    std::unique_ptr<juce::Label> nowPlayingLabel;
    std::unique_ptr<juce::Label> nowPlayingTrackName;
    void updateNowPlayingDisplay();

    // Standalone mode helpers
    void setupStandaloneMode();
    void onTransportPlayPause();
    void onTransportSeek(double normalizedPosition);
    void onCardCountChanged(int newCount);
    void onCardFileDropped(int cardIndex, const juce::File& file);
    void onCardRemoveFile(int cardIndex);
    void updateTransportFromEngine();

    // Preset helpers
    void setupPresetUI();
    void onSavePreset();
    void onLoadPreset(int presetIndex);
    void onDeletePreset();
    void refreshPresetList();
    StandalonePresetManager::PresetData getCurrentPresetData() const;
    void applyPresetData(const StandalonePresetManager::PresetData& data);

    // Batch import helper
    void onImportFiles();

    //==========================================================================
    // Q&A countdown state
    juce::int64 lastCountdownTick = 0;  // Timestamp of last countdown tick

    // Auto-advance timer for Guess mode
    juce::int64 autoAdvanceStartTime = 0;  // When countdown started (0 = disabled)
    int autoAdvanceCountdown = 0;          // Current countdown value (3, 2, 1, 0)
    static constexpr int kAutoAdvanceSeconds = 3;  // 3 seconds countdown

    //==========================================================================
    // Developer mode panel (toggle with 'D' key)
    bool devPanelVisible = false;
    std::unique_ptr<juce::Component> devPanel;
    std::unique_ptr<juce::Slider> trackCountSlider;
    std::unique_ptr<juce::Label> trackCountLabel;
    std::unique_ptr<juce::TextButton> applyButton;
    void setupDevPanel();
    void applyTrackCount(int count);
    void toggleDevPanel();

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
    void onQAQuestionsChanged(int questions);

    // Event handlers - ResultsPanel
    void onSubmitGuesses();

    //==========================================================================
    // Helpers
    std::vector<std::string> getTrackNames() const;
    void buildStarsResults();
    void buildGuessResults();
    void buildQAResults();
    int findCardIdAtTablePosition(int tablePosition);  // Find card ID based on table position

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BlindCardEditor)
};

} // namespace BlindCard
