/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

/*
  ==============================================================================

    LocalizationManager.h
    Created: 2026-02-02
    Author:  BlindCard

    Multi-language localization system for BlindCard UI.
    Supports: English, Traditional Chinese (繁體中文), Simplified Chinese (简体中文),
              Japanese (日本語), Korean (한국어)

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <map>
#include <string>

namespace BlindCard
{

//==============================================================================
/**
 * Supported languages
 */
enum class Language
{
    English,
    TraditionalChinese,
    SimplifiedChinese,
    Japanese,
    Korean
};

//==============================================================================
/**
 * String keys for all localizable UI text
 */
namespace StringKey
{
    // Header Bar
    constexpr const char* NowPlaying = "now_playing";
    constexpr const char* NoFileLoaded = "no_file_loaded";

    // Mode Selector
    constexpr const char* ModeStars = "mode_stars";
    constexpr const char* ModeGuess = "mode_guess";
    constexpr const char* ModeQA = "mode_qa";

    // Control Panel - Buttons
    constexpr const char* ButtonShuffle = "btn_shuffle";
    constexpr const char* ButtonReveal = "btn_reveal";
    constexpr const char* ButtonReset = "btn_reset";
    constexpr const char* ButtonNextRound = "btn_next_round";
    constexpr const char* ButtonSubmit = "btn_submit";

    // Control Panel - Labels
    constexpr const char* LabelRounds = "label_rounds";
    constexpr const char* LabelAutoGain = "label_auto_gain";
    constexpr const char* LabelQuestions = "label_questions";
    constexpr const char* LabelCards = "label_cards";

    // Control Panel - Status
    constexpr const char* StatusReady = "status_ready";
    constexpr const char* StatusShuffled = "status_shuffled";
    constexpr const char* StatusRevealed = "status_revealed";
    constexpr const char* StatusRound = "status_round";
    constexpr const char* StatusOf = "status_of";
    constexpr const char* StatusDetecting = "status_detecting";
    constexpr const char* StatusWaiting = "status_waiting";

    // Results Panel
    constexpr const char* ResultsTitle = "results_title";
    constexpr const char* ResultsEmpty = "results_empty";
    constexpr const char* ResultsTrack = "results_track";
    constexpr const char* ResultsScore = "results_score";
    constexpr const char* ResultsAverage = "results_average";
    constexpr const char* ResultsCorrect = "results_correct";
    constexpr const char* ResultsIncorrect = "results_incorrect";
    constexpr const char* ResultsTotal = "results_total";
    constexpr const char* ResultsCurrentRound = "results_current_round";    // "CURRENT ROUND"
    constexpr const char* ResultsFinalResults = "results_final_results";    // "FINAL RESULTS"
    constexpr const char* ResultsAnswerQuestions = "results_answer_questions";  // "Answer questions to see results"

    // Question Banner (Q&A mode)
    constexpr const char* QAQuestion = "qa_question";
    constexpr const char* QASelectCard = "qa_select_card";
    constexpr const char* QAWhichBetter = "qa_which_better";
    constexpr const char* QAPreference = "qa_preference";
    constexpr const char* QAWhichCardIs = "qa_which_card_is";      // "Which card is [name]?"
    constexpr const char* QAWhichOneIs = "qa_which_one_is";        // "Which one is [name]?"
    constexpr const char* QARemaining = "qa_remaining";            // "remaining"
    constexpr const char* QAQuestionProgress = "qa_question_progress";  // "Question"
    constexpr const char* QAAnswer = "qa_answer";                  // "Answer:"
    constexpr const char* QAScoreLabel = "qa_score_label";         // "Score:"
    constexpr const char* QAPressResetToPlayAgain = "qa_press_reset";  // "Press Reset to play again"

    // Poker Card
    constexpr const char* CardDropHere = "card_drop_here";
    constexpr const char* CardDragFile = "card_drag_file";
    constexpr const char* CardTrack = "card_track";
    constexpr const char* CardSelect = "card_select";           // "Select" button

    // Card Count Control
    constexpr const char* CardCountLabel = "card_count_label";

    // Settings Panel
    constexpr const char* SettingsTitle = "settings_title";
    constexpr const char* SettingsLanguage = "settings_language";
    constexpr const char* SettingsTheme = "settings_theme";
    constexpr const char* SettingsThemeLight = "settings_theme_light";
    constexpr const char* SettingsThemeDark = "settings_theme_dark";
    constexpr const char* SettingsClose = "settings_close";

    // About Dialog
    constexpr const char* AboutTitle = "about_title";
    constexpr const char* AboutFreeSoftware = "about_free_software";
    constexpr const char* AboutNoLicense = "about_no_license";
    constexpr const char* AboutVersion = "about_version";
    constexpr const char* AboutDeveloper = "about_developer";
    constexpr const char* AboutVisitWebsite = "about_visit_website";

    // Update Checker
    constexpr const char* UpdateAvailable = "update_available";
    constexpr const char* UpdateUpToDate = "update_up_to_date";
    constexpr const char* UpdateDownload = "update_download";

    // Quick Start Guide
    constexpr const char* GuideTitle = "guide_title";
    constexpr const char* GuideGotIt = "guide_got_it";
    constexpr const char* GuideDescription = "guide_description";

    // Steps (full detailed text)
    constexpr const char* GuideStep1Full = "guide_step1_full";
    constexpr const char* GuideStep2Intro = "guide_step2_intro";
    constexpr const char* GuideStep3Full = "guide_step3_full";
    constexpr const char* GuideStep4Full = "guide_step4_full";
    constexpr const char* GuideStep5Full = "guide_step5_full";
    constexpr const char* GuideStep6Full = "guide_step6_full";

    // Mode descriptions in guide
    constexpr const char* GuideModeStarsDesc = "guide_mode_stars_desc";
    constexpr const char* GuideModeGuessDesc = "guide_mode_guess_desc";
    constexpr const char* GuideModeQADesc = "guide_mode_qa_desc";

    // Tip box
    constexpr const char* GuideTip = "guide_tip";
    constexpr const char* GuideTipText1 = "guide_tip_text1";
    constexpr const char* GuideTipText2 = "guide_tip_text2";

    // Keyboard shortcuts
    constexpr const char* GuideKeyboardShortcuts = "guide_keyboard_shortcuts";
    constexpr const char* GuideShortcutSelectCard = "guide_shortcut_select_card";
    constexpr const char* GuideShortcutNextPrev = "guide_shortcut_next_prev";
    constexpr const char* GuideShortcutBrackets = "guide_shortcut_brackets";
    constexpr const char* GuideShortcutArrows = "guide_shortcut_arrows";

    // Dialogs
    constexpr const char* DialogSavePreset = "dialog_save_preset";
    constexpr const char* DialogEnterName = "dialog_enter_name";
    constexpr const char* DialogSave = "dialog_save";
    constexpr const char* DialogCancel = "dialog_cancel";
    constexpr const char* DialogDelete = "dialog_delete";
    constexpr const char* DialogConfirmDelete = "dialog_confirm_delete";
    constexpr const char* DialogYes = "dialog_yes";
    constexpr const char* DialogNo = "dialog_no";

    // Import
    constexpr const char* ImportFiles = "import_files";
    constexpr const char* ImportSuccess = "import_success";
    constexpr const char* ImportFailed = "import_failed";
    constexpr const char* ImportFilesLoaded = "import_files_loaded";

    // Transport Bar
    constexpr const char* TransportPlay = "transport_play";
    constexpr const char* TransportPause = "transport_pause";
    constexpr const char* TransportStop = "transport_stop";

    // Presets
    constexpr const char* PresetDefault = "preset_default";
    constexpr const char* PresetSave = "preset_save";
    constexpr const char* PresetDelete = "preset_delete";
    constexpr const char* PresetSelect = "preset_select";

    // Star Rating
    constexpr const char* RatingLabel = "rating_label";

    // Guess Dropdown
    constexpr const char* GuessSelect = "guess_select";
    constexpr const char* GuessNone = "guess_none";
}

//==============================================================================
/**
 * LocalizationManager - Singleton class for managing UI translations
 */
class LocalizationManager
{
public:
    //==========================================================================
    /** Get the singleton instance */
    static LocalizationManager& getInstance()
    {
        static LocalizationManager instance;
        return instance;
    }

    //==========================================================================
    /** Get current language */
    Language getCurrentLanguage() const { return currentLanguage; }

    /** Set current language */
    void setLanguage(Language lang)
    {
        if (currentLanguage != lang)
        {
            currentLanguage = lang;
            notifyListeners();
        }
    }

    /** Get language display name */
    juce::String getLanguageDisplayName(Language lang) const
    {
        switch (lang)
        {
            case Language::English:            return "English";
            case Language::TraditionalChinese: return juce::CharPointer_UTF8("\xe7\xb9\x81\xe9\xab\x94\xe4\xb8\xad\xe6\x96\x87");  // 繁體中文
            case Language::SimplifiedChinese:  return juce::CharPointer_UTF8("\xe7\xae\x80\xe4\xbd\x93\xe4\xb8\xad\xe6\x96\x87");  // 简体中文
            case Language::Japanese:           return juce::CharPointer_UTF8("\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e");  // 日本語
            case Language::Korean:             return juce::CharPointer_UTF8("\xed\x95\x9c\xea\xb5\xad\xec\x96\xb4");  // 한국어
            default:                           return "English";
        }
    }

    /** Get localized string by key */
    juce::String getString(const char* key) const
    {
        auto& langStrings = getStringsForLanguage(currentLanguage);
        auto it = langStrings.find(key);
        if (it != langStrings.end())
            return it->second;

        // Fallback to English
        auto& englishStrings = getStringsForLanguage(Language::English);
        auto fallback = englishStrings.find(key);
        if (fallback != englishStrings.end())
            return fallback->second;

        // Return key if not found
        return juce::String(key);
    }

    //==========================================================================
    /** Listener interface for language changes */
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void languageChanged() = 0;
    };

    void addListener(Listener* listener)
    {
        listeners.push_back(listener);
    }

    void removeListener(Listener* listener)
    {
        listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
    }

private:
    //==========================================================================
    LocalizationManager() : currentLanguage(Language::English)
    {
        initializeStrings();
    }

    void notifyListeners()
    {
        for (auto* listener : listeners)
            listener->languageChanged();
    }

    const std::map<std::string, juce::String>& getStringsForLanguage(Language lang) const
    {
        switch (lang)
        {
            case Language::TraditionalChinese: return stringsZhTw;
            case Language::SimplifiedChinese:  return stringsZhCn;
            case Language::Japanese:           return stringsJa;
            case Language::Korean:             return stringsKo;
            default:                           return stringsEn;
        }
    }

    void initializeStrings();

    //==========================================================================
    Language currentLanguage;
    std::vector<Listener*> listeners;

    std::map<std::string, juce::String> stringsEn;
    std::map<std::string, juce::String> stringsZhTw;
    std::map<std::string, juce::String> stringsZhCn;
    std::map<std::string, juce::String> stringsJa;
    std::map<std::string, juce::String> stringsKo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LocalizationManager)
};

//==============================================================================
/** Convenience macro for getting localized strings */
#define LOCALIZE(key) BlindCard::LocalizationManager::getInstance().getString(BlindCard::StringKey::key)

} // namespace BlindCard
