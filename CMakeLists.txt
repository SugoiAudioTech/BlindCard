# Copyright (c) 2026 Sugoi Audio Tech
# SPDX-License-Identifier: AGPL-3.0-only

cmake_minimum_required(VERSION 3.22)

project(BlindCard VERSION 1.0.0)

# Fetch JUCE automatically
include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.12
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(BlindCard
    COMPANY_NAME "Sugoi"
    PLUGIN_MANUFACTURER_CODE Sgoi
    PLUGIN_CODE Blcd
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Blind Card"
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon_1024.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon_256.png"
    COPY_PLUGIN_AFTER_BUILD TRUE)

# Binary assets (card images, fonts, and logo)
juce_add_binary_data(BlindCardAssets SOURCES
    assets/cards/card-back.png
    assets/sugoi-logo.png
    assets/fonts/NotoSansTC-Regular.otf
    assets/fonts/NotoSansTC-Medium.otf
    assets/fonts/NotoSansTC-Bold.otf
    assets/fonts/BebasNeue-Regular.ttf
    assets/fonts/Cinzel-Regular.ttf
    assets/fonts/Cinzel-Bold.ttf)

target_sources(BlindCard
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/Core/Types.h
        Source/Core/BlindCardManager.h
        Source/Core/BlindCardManager.cpp

        # Standalone Audio Engine
        Source/Standalone/StandaloneAudioEngine.h
        Source/Standalone/StandaloneAudioEngine.cpp
        Source/Standalone/StandalonePresetManager.h
        Source/Standalone/StandalonePresetManager.cpp

        # UI System (Casino Poker Theme)
        Source/UI/Theme/ThemeColors.h
        Source/UI/Theme/ThemeManager.h
        Source/UI/Theme/ThemeManager.cpp
        Source/UI/Theme/FontManager.h
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
        Source/UI/Components/QuickStartGuide.h
        Source/UI/Components/QuickStartGuide.cpp
        Source/UI/Components/SettingsPanel.h
        Source/UI/Components/SettingsPanel.cpp
        Source/UI/Components/TransportBar.h
        Source/UI/Components/TransportBar.cpp
        Source/UI/Components/CardCountControl.h
        Source/UI/Components/CardCountControl.cpp

        # Localization
        Source/UI/Localization/LocalizationManager.h
        Source/UI/Localization/LocalizationManager.cpp)

target_compile_definitions(BlindCard
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        # Standalone audio settings
        JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=0
        JUCE_STANDALONE_FILTER_WINDOW_USE_KIOSK_MODE=0)

target_compile_features(BlindCard PRIVATE cxx_std_17)

target_link_libraries(BlindCard
    PRIVATE
        BlindCardAssets
        juce::juce_audio_utils
        juce::juce_audio_formats
        juce::juce_events
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
