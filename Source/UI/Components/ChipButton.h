/*
  ==============================================================================

    ChipButton.h
    Created: 2026-01-19
    Author:  BlindCard

    3D poker chip button component with animated hover/press effects.
    Used in ControlPanel for SHUFFLE, REVEAL, RESET, NEXT actions.

    Size: 64x64px chip + label below = ~84px total height

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include "../Theme/ThemeManager.h"
#include "../Animation/AnimatedValue.h"

namespace BlindCard
{

//==============================================================================
/**
 * Variant types for ChipButton, each with distinct colors.
 */
enum class ChipVariant
{
    Red,    // SHUFFLE button - gradient #8B0000 -> #CC0000
    Gold,   // NEXT ROUND button - gradient #8B7300 -> #DAA520
    Black,  // RESET button - gradient #1A1A1A -> #333333
    Blue    // REVEAL button - gradient #1A4A7A -> #2E6EB0
};

//==============================================================================
/**
 * ChipButton displays a 3D poker chip with animated hover/press effects.
 *
 * Visual structure (paint order):
 * 1. Outer ring: radial gradient, base color
 * 2. 12 cream stripes (#F5F5DC) around edge
 * 3. Middle black ring (#000000)
 * 4. Gold border line (1.5px, #FFD700)
 * 5. Inner circle: darker version of base color
 * 6. Icon: white, centered
 * 7. Label: below the chip
 *
 * States:
 * - Idle: bottom shadow
 * - Hover: outer glow (color-based), translate up 2px
 * - Pressed: scale 95%, reduced shadow
 * - Disabled: opacity 0.4
 *
 * Usage:
 *   ChipButton shuffleButton("SHUFFLE", ChipVariant::Red);
 *   shuffleButton.setIcon(shuffleIcon); // Unicode or custom path
 *   shuffleButton.onClick = [this]() { handleShuffle(); };
 *   shuffleButton.setEnabled(true);
 */
class ChipButton : public juce::Component,
                   public juce::ChangeListener
{
public:
    //==========================================================================
    /**
     * Constructor.
     * @param labelText The text label displayed below the chip
     * @param variant The color variant (Red, Gold, Black, Blue)
     */
    ChipButton(const juce::String& labelText, ChipVariant variant);

    /** Destructor */
    ~ChipButton() override;

    //==========================================================================
    /** Callback triggered when the button is clicked */
    std::function<void()> onClick;

    //==========================================================================
    /**
     * Sets the enabled state of the button.
     * When disabled, the button is semi-transparent and non-interactive.
     * @param shouldBeEnabled true to enable, false to disable
     */
    void setEnabled(bool shouldBeEnabled);

    /** Returns true if the button is enabled */
    bool isEnabled() const { return enabledState; }

    /**
     * Sets the icon to display in the center of the chip.
     * @param iconText Unicode character or text to display
     */
    void setIcon(const juce::String& iconText);

    /**
     * Sets the label text below the chip.
     * @param newLabel The new label text
     */
    void setLabel(const juce::String& newLabel);

    /**
     * Sets the chip variant (color scheme).
     * @param newVariant The new variant to use
     */
    void setVariant(ChipVariant newVariant);

    /** Returns the current variant */
    ChipVariant getVariant() const { return variant; }

    //==========================================================================
    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

    // Timer callback for animation updates
    void updateAnimations();

    // ChangeListener override (for ThemeManager)
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    //==========================================================================
    /** Standard dimensions */
    static constexpr int kChipSize = 64;
    static constexpr int kLabelHeight = 20;
    static constexpr int kTotalHeight = kChipSize + kLabelHeight;
    static constexpr int kTotalWidth = kChipSize;

private:
    //==========================================================================
    // State
    juce::String label;
    juce::String icon;
    ChipVariant variant;
    bool enabledState = true;
    bool isHovered = false;
    bool isPressed = false;

    //==========================================================================
    // Animation values (0.0 to 1.0)
    AnimatedValue hoverProgress;   // Controls hover glow and translation
    AnimatedValue pressProgress;   // Controls scale and shadow reduction

    // Animation constants
    static constexpr float kAnimationDurationMs = 150.0f;

    //==========================================================================
    // Timer for animations
    class AnimationTimer : public juce::Timer
    {
    public:
        AnimationTimer(ChipButton& owner) : button(owner) {}
        void timerCallback() override { button.updateAnimations(); }
    private:
        ChipButton& button;
    };
    std::unique_ptr<AnimationTimer> animationTimer;
    juce::int64 lastUpdateTime = 0;

    //==========================================================================
    // Drawing helpers
    void drawChipShadow(juce::Graphics& g, juce::Rectangle<float> chipBounds, float shadowAmount);
    void drawChipGlow(juce::Graphics& g, juce::Rectangle<float> chipBounds, float glowAmount);
    void drawOuterRing(juce::Graphics& g, juce::Rectangle<float> chipBounds);
    void drawCreamStripes(juce::Graphics& g, juce::Point<float> center, float radius);
    void drawMiddleRing(juce::Graphics& g, juce::Point<float> center, float outerRadius, float innerRadius);
    void drawGoldBorder(juce::Graphics& g, juce::Point<float> center, float radius);
    void drawInnerCircle(juce::Graphics& g, juce::Point<float> center, float radius);
    void drawIcon(juce::Graphics& g, juce::Point<float> center, float radius);
    void drawLabel(juce::Graphics& g, juce::Rectangle<float> labelBounds);

    // Color helpers
    juce::Colour getBaseColor() const;
    juce::Colour getDarkColor() const;
    juce::Colour getGlowColor() const;

    // Utility
    void startAnimationTimer();
    void stopAnimationTimerIfIdle();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChipButton)
};

} // namespace BlindCard
