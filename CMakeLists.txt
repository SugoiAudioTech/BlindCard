cmake_minimum_required(VERSION 3.22)

project(BlindCard VERSION 1.0.0)

add_subdirectory(/Users/Mac/JUCE ${CMAKE_BINARY_DIR}/JUCE)

juce_add_plugin(BlindCard
    COMPANY_NAME "Sugoi"
    PLUGIN_MANUFACTURER_CODE Sgoi
    PLUGIN_CODE Blcd
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Blind Card"
    COPY_PLUGIN_AFTER_BUILD TRUE)

# Binary assets (glow effects and card images)
juce_add_binary_data(BlindCardAssets SOURCES
    assets/glow/glow_warm@1x.png
    assets/glow/glow_warm@2x.png
    assets/glow/glow_cool@1x.png
    assets/glow/glow_cool@2x.png
    assets/glow/compact/glow_warm_compact@1x.png
    assets/glow/compact/glow_warm_compact@2x.png
    assets/glow/compact/glow_cool_compact@1x.png
    assets/glow/compact/glow_cool_compact@2x.png
    assets/cards/card-back.png)

target_sources(BlindCard
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/Core/Types.h
        Source/Core/BlindCardManager.h
        Source/Core/BlindCardManager.cpp

        # New UI System (Casino Poker Theme)
        Source/UI/Theme/ThemeColors.h
        Source/UI/Theme/ThemeManager.h
        Source/UI/Theme/ThemeManager.cpp
        Source/UI/Animation/AnimatedValue.h
        Source/UI/BlindCardEditor.h
        Source/UI/BlindCardEditor.cpp
        Source/UI/Components/HeaderBar.h
        Source/UI/Components/HeaderBar.cpp
        Source/UI/Components/ModeSelector.h
        Source/UI/Components/ModeSelector.cpp
        Source/UI/Components/ChipButton.h
        Source/UI/Components/ChipButton.cpp
        Source/UI/Components/ChipStack.h
        Source/UI/Components/ChipStack.cpp
        Source/UI/Components/StarRating.h
        Source/UI/Components/StarRating.cpp
        Source/UI/Components/GuessDropdown.h
        Source/UI/Components/GuessDropdown.cpp
        Source/UI/Components/PokerCard.h
        Source/UI/Components/PokerCard.cpp
        Source/UI/Components/PokerTable.h
        Source/UI/Components/PokerTable.cpp
        Source/UI/Components/QuestionBanner.h
        Source/UI/Components/QuestionBanner.cpp
        Source/UI/Components/ControlPanel.h
        Source/UI/Components/ControlPanel.cpp
        Source/UI/Components/ResultsPanel.h
        Source/UI/Components/ResultsPanel.cpp

        # Legacy UI (to be removed)
        Source/UI/CardComponent.h
        Source/UI/CardComponent.cpp
        Source/UI/ColorUtils.h
        Source/UI/DebugPanel.h
        Source/UI/GlowAssets.h)

target_compile_definitions(BlindCard
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_compile_features(BlindCard PRIVATE cxx_std_17)

target_link_libraries(BlindCard
    PRIVATE
        BlindCardAssets
        juce::juce_audio_utils
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
