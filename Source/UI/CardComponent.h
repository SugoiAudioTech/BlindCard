#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Core/Types.h"
#include "ColorUtils.h"
#include "GlowAssets.h"

namespace blindcard
{

/**
 * Selection state enum
 * Used to distinguish primary/secondary/unselected
 */
enum class SelectionState
{
    None,      // Not selected
    Primary,   // Primary selection: alpha 0.8
    Secondary  // Secondary selection: alpha 0.38-0.40
};

/**
 * Flip state enum
 */
enum class FlipState
{
    FaceDown,   // Face down
    Flipping,   // Flip animation in progress
    FaceUp      // Face up
};

class CardComponent final : public juce::Component,
                            public juce::Timer,
                            public juce::KeyListener
{
public:
    CardComponent();

    void setCard (const CardSlot& card, int currentRound, bool isSelected, GamePhase phase, RatingMode ratingMode, const QAState* qaState = nullptr);
    void clearCard();

    std::function<void(int)> onClicked;
    std::function<void(int, int)> onRated;
    std::function<void(int, float)> onGainChanged;
    std::function<void(int, const juce::String&)> onNoteChanged;
    std::function<void(int, int)> onGuessChanged;
    std::function<void(int)> onQASubmit;

    void setTrackNames (const juce::StringArray& names);

    // Selection state control (primary/secondary/none)
    void setSelectionState (SelectionState state);
    SelectionState getSelectionState() const { return selectionState; }

    // Flip animation control
    void startFlip();
    void setFlipped (bool flipped, bool animate = true);
    bool isFlipped() const { return flipState == FlipState::FaceUp; }
    bool isFlipping() const { return flipState == FlipState::Flipping; }

    // Glow flash control (triggered when flipping)
    void triggerGlowFlash();

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    bool keyPressed (const juce::KeyPress& key) override;

    // KeyListener - Intercept spacebar from TextEditor, allow DAW to play/pause
    bool keyPressed (const juce::KeyPress& key, juce::Component* originatingComponent) override;

    // Timer callback for animation updates
    void timerCallback() override;

    // Debug parameter access (for Debug Panel use)
    struct DebugParams
    {
        float transitionDuration = 150.0f;   // Selection transition duration ms
        float flipDuration = 300.0f;         // Flip animation duration ms
        float flashDuration = 200.0f;        // Glow flash duration ms
        float glowAlphaPrimary = 0.8f;       // Primary selection glow alpha
        float glowAlphaSecondary = 0.38f;    // Secondary selection glow alpha (deprecated, use MultiCardConfig.subAlpha)
        float glowAlphaIdle = 0.3f;          // Idle selection glow alpha
        float glowScale = 1.3f;              // Glow size multiplier
        bool useEaseOut = true;              // Use ease-out curve
    };

    static DebugParams& getDebugParams()
    {
        static DebugParams params;
        return params;
    }

    /**
     * Multi-card scene dynamic configuration
     * Auto-adjusts spacing, glow radius, secondary alpha based on card count
     * Maintains ~2:1 visual distinction between primary and secondary selection
     */
    struct MultiCardConfig
    {
        float spacingScale;      // Spacing scale ratio (1.0 = 100%)
        float glowRadiusScale;   // Glow radius scale ratio (1.0 = 100%)
        float subAlpha;          // Secondary selection alpha
    };

    /**
     * Get dynamic configuration based on card count
     * 4-6 cards: 100%/85% spacing, 100%/85% glow, 0.38 secondary
     * 7-8 cards: 80%/75% spacing, 82%/80% glow, 0.40 secondary
     */
    static MultiCardConfig getMultiCardConfig (int numCards)
    {
        if (numCards <= 4)
            return { 1.0f, 1.0f, 0.38f };
        else if (numCards <= 6)
            return { 0.85f, 0.85f, 0.38f };
        else if (numCards == 7)
            return { 0.80f, 0.82f, 0.40f };
        else
            return { 0.75f, 0.80f, 0.40f };
    }

    // Set current multi-card configuration (called by Editor)
    void setMultiCardConfig (const MultiCardConfig& config);
    const MultiCardConfig& getCurrentMultiCardConfig() const { return currentMultiCardConfig; }

private:
    CardSlot cardData;
    bool hasCard = false;
    int round = 0;
    bool selected = false;
    GamePhase currentPhase = GamePhase::Setup;
    RatingMode currentRatingMode = RatingMode::Stars;

    // Selection state
    SelectionState selectionState = SelectionState::None;

    // Flip animation state
    FlipState flipState = FlipState::FaceDown;
    AnimatedValue flipProgress { 0.0f };  // 0 = back, 1 = front

    // Glow animation state
    AnimatedValue glowAlpha { 0.0f };     // Current glow alpha
    bool isFlashActive = false;           // Whether flash is active
    float flashPhase = 0.0f;              // Flash animation progress (0-1)

    // Multi-card scene configuration
    MultiCardConfig currentMultiCardConfig { 1.0f, 1.0f, 0.38f };

    // Glow colors
    juce::Colour glowColorWarm { 0xFFFFD700 };   // Warm gold: for flip flash
    juce::Colour glowColorCool { 0xFFFFF8E1 };   // Cool white-gold: for selection state

    // Animation timing
    juce::int64 lastUpdateTime = 0;

    juce::TextButton starButtons[5];
    juce::Slider gainSlider;
    juce::Label lufsLabel;
    juce::TextEditor noteEditor;
    juce::ComboBox guessComboBox;
    juce::StringArray trackNames;

    // Q&A 模式
    juce::TextButton selectButton;
    QAState::FeedbackState qaFeedbackState = QAState::FeedbackState::None;
    bool isQATarget = false;
    bool wasQASelected = false;

    void updateStars();
    void updateGainDisplay();
    void updateNoteEditor();
    void updateGuessComboBox();
    void updateQAButton();

    // Paint functions (three-layer architecture)
    void paintCardBase (juce::Graphics& g, const juce::Rectangle<float>& bounds);
    void paintGlowLayer (juce::Graphics& g, const juce::Rectangle<float>& bounds);
    void paintGoldBorder (juce::Graphics& g, const juce::Rectangle<float>& bounds);
    void paintCardContent (juce::Graphics& g, const juce::Rectangle<float>& bounds);

    // Calculate flip transform
    juce::AffineTransform getFlipTransform (const juce::Rectangle<float>& bounds) const;

    // Update glow target alpha
    void updateGlowTarget();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CardComponent)
};

} // namespace blindcard
