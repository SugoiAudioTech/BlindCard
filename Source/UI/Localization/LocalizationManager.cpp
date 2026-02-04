/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include "LocalizationManager.h"

namespace BlindCard
{

void LocalizationManager::initializeStrings()
{
    //==========================================================================
    // ENGLISH
    //==========================================================================

    // Header Bar
    stringsEn[StringKey::NowPlaying] = "Now Playing:";
    stringsEn[StringKey::NoFileLoaded] = "No file loaded";

    // Mode Selector
    stringsEn[StringKey::ModeStars] = "Stars";
    stringsEn[StringKey::ModeGuess] = "Guess";
    stringsEn[StringKey::ModeQA] = "Q&A";

    // Control Panel - Buttons
    stringsEn[StringKey::ButtonShuffle] = "SHUFFLE";
    stringsEn[StringKey::ButtonReveal] = "REVEAL";
    stringsEn[StringKey::ButtonReset] = "RESET";
    stringsEn[StringKey::ButtonNextRound] = "NEXT ROUND";
    stringsEn[StringKey::ButtonSubmit] = "SUBMIT";

    // Control Panel - Labels
    stringsEn[StringKey::LabelRounds] = "Rounds";
    stringsEn[StringKey::LabelAutoGain] = "Auto Gain";
    stringsEn[StringKey::LabelQuestions] = "Questions";
    stringsEn[StringKey::LabelCards] = "Cards";

    // Control Panel - Status
    stringsEn[StringKey::StatusReady] = "Ready";
    stringsEn[StringKey::StatusShuffled] = "Shuffled";
    stringsEn[StringKey::StatusRevealed] = "Revealed";
    stringsEn[StringKey::StatusRound] = "Round";
    stringsEn[StringKey::StatusOf] = "of";
    stringsEn[StringKey::StatusDetecting] = "Detecting...";
    stringsEn[StringKey::StatusWaiting] = "Waiting...";

    // Results Panel
    stringsEn[StringKey::ResultsTitle] = "Results";
    stringsEn[StringKey::ResultsEmpty] = "No results yet";
    stringsEn[StringKey::ResultsTrack] = "Track";
    stringsEn[StringKey::ResultsScore] = "Score";
    stringsEn[StringKey::ResultsAverage] = "Average";
    stringsEn[StringKey::ResultsCorrect] = "Correct";
    stringsEn[StringKey::ResultsIncorrect] = "Incorrect";
    stringsEn[StringKey::ResultsTotal] = "Total";

    // Question Banner (Q&A mode)
    stringsEn[StringKey::QAQuestion] = "Question";
    stringsEn[StringKey::QASelectCard] = "Select a card";
    stringsEn[StringKey::QAWhichBetter] = "Which sounds better?";
    stringsEn[StringKey::QAPreference] = "Your preference";

    // Poker Card
    stringsEn[StringKey::CardDropHere] = "Drop audio file here";
    stringsEn[StringKey::CardDragFile] = "Drag & drop audio";
    stringsEn[StringKey::CardTrack] = "Track";

    // Card Count Control
    stringsEn[StringKey::CardCountLabel] = "Number of Cards";

    // Settings Panel
    stringsEn[StringKey::SettingsTitle] = "Settings";
    stringsEn[StringKey::SettingsLanguage] = "Language";
    stringsEn[StringKey::SettingsTheme] = "Theme";
    stringsEn[StringKey::SettingsThemeLight] = "Light";
    stringsEn[StringKey::SettingsThemeDark] = "Dark";
    stringsEn[StringKey::SettingsClose] = "Close";

    // About Dialog
    stringsEn[StringKey::AboutTitle] = "About";
    stringsEn[StringKey::AboutFreeSoftware] = "It's Free Software";
    stringsEn[StringKey::AboutNoLicense] = "No license key required";
    stringsEn[StringKey::AboutVersion] = "Version:";
    stringsEn[StringKey::AboutDeveloper] = "Developer:";
    stringsEn[StringKey::AboutVisitWebsite] = "Visit Official Website";

    // Quick Start Guide
    stringsEn[StringKey::GuideTitle] = "Quick Start Guide";
    stringsEn[StringKey::GuideGotIt] = "Got it!";
    stringsEn[StringKey::GuideDescription] = "Blind Card lets you compare audio tracks without knowing which is which - eliminating bias from your decisions.";

    // Steps (full detailed text)
    stringsEn[StringKey::GuideStep1Full] = "Insert Blind Card on each track you want to compare (2-8 tracks)";
    stringsEn[StringKey::GuideStep2Intro] = "Choose a rating mode:";
    stringsEn[StringKey::GuideStep3Full] = "Click \"SHUFFLE\" to randomize and hide track identities";
    stringsEn[StringKey::GuideStep4Full] = "Click cards to listen and rate (or use arrow keys)";
    stringsEn[StringKey::GuideStep5Full] = "Click \"REVEAL\" to see results and actual track names";
    stringsEn[StringKey::GuideStep6Full] = "Click \"RESET\" to start a new test";

    // Mode descriptions in guide
    stringsEn[StringKey::GuideModeStarsDesc] = "Rate each track from 1-5 stars";
    stringsEn[StringKey::GuideModeGuessDesc] = "Identify which plugin/mix is on each card";
    stringsEn[StringKey::GuideModeQADesc] = "Find a specific track when prompted";

    // Tip box
    stringsEn[StringKey::GuideTip] = "Tip";
    stringsEn[StringKey::GuideTipText1] = "Enable \"Auto Gain\" to level-match all tracks,";
    stringsEn[StringKey::GuideTipText2] = "ensuring fair comparison without volume bias.";

    // Keyboard shortcuts
    stringsEn[StringKey::GuideKeyboardShortcuts] = "Keyboard Shortcuts";
    stringsEn[StringKey::GuideShortcutSelectCard] = "Select card directly";
    stringsEn[StringKey::GuideShortcutNextPrev] = "Next / Previous card";
    stringsEn[StringKey::GuideShortcutBrackets] = "Previous / Next card";
    stringsEn[StringKey::GuideShortcutArrows] = "Navigate cards (may not work in Logic Pro)";

    // Dialogs
    stringsEn[StringKey::DialogSavePreset] = "Save Preset";
    stringsEn[StringKey::DialogEnterName] = "Enter preset name:";
    stringsEn[StringKey::DialogSave] = "Save";
    stringsEn[StringKey::DialogCancel] = "Cancel";
    stringsEn[StringKey::DialogDelete] = "Delete";
    stringsEn[StringKey::DialogConfirmDelete] = "Delete this preset?";
    stringsEn[StringKey::DialogYes] = "Yes";
    stringsEn[StringKey::DialogNo] = "No";

    // Import
    stringsEn[StringKey::ImportFiles] = "Import Files";
    stringsEn[StringKey::ImportSuccess] = "Import successful";
    stringsEn[StringKey::ImportFailed] = "Import failed";
    stringsEn[StringKey::ImportFilesLoaded] = "files loaded";

    // Transport Bar
    stringsEn[StringKey::TransportPlay] = "Play";
    stringsEn[StringKey::TransportPause] = "Pause";
    stringsEn[StringKey::TransportStop] = "Stop";

    // Presets
    stringsEn[StringKey::PresetDefault] = "Default";
    stringsEn[StringKey::PresetSave] = "Save";
    stringsEn[StringKey::PresetDelete] = "Delete";
    stringsEn[StringKey::PresetSelect] = "Select Preset";

    // Star Rating
    stringsEn[StringKey::RatingLabel] = "Rating";

    // Guess Dropdown
    stringsEn[StringKey::GuessSelect] = "Select track...";
    stringsEn[StringKey::GuessNone] = "None";

    //==========================================================================
    // TRADITIONAL CHINESE (繁體中文)
    //==========================================================================

    // Header Bar
    stringsZhTw[StringKey::NowPlaying] = juce::CharPointer_UTF8("\xe6\xad\xa3\xe5\x9c\xa8\xe6\x92\xad\xe6\x94\xbe\xef\xbc\x9a");  // 正在播放：
    stringsZhTw[StringKey::NoFileLoaded] = juce::CharPointer_UTF8("\xe6\x9c\xaa\xe8\xbc\x89\xe5\x85\xa5\xe6\xaa\x94\xe6\xa1\x88");  // 未載入檔案

    // Mode Selector
    stringsZhTw[StringKey::ModeStars] = juce::CharPointer_UTF8("\xe8\xa9\x95\xe5\x88\x86\xe6\xa8\xa1\xe5\xbc\x8f");  // 評分模式
    stringsZhTw[StringKey::ModeGuess] = juce::CharPointer_UTF8("\xe9\x85\x8d\xe5\xb0\x8d");  // 配對
    stringsZhTw[StringKey::ModeQA] = juce::CharPointer_UTF8("\xe4\xbd\xa0\xe5\x95\x8f\xe6\x88\x91\xe7\xad\x94");  // 你問我答

    // Control Panel - Buttons
    stringsZhTw[StringKey::ButtonShuffle] = juce::CharPointer_UTF8("\xe6\xb4\x97\xe7\x89\x8c");  // 洗牌
    stringsZhTw[StringKey::ButtonReveal] = juce::CharPointer_UTF8("\xe9\x96\x8b\xe7\x89\x8c");  // 開牌
    stringsZhTw[StringKey::ButtonReset] = juce::CharPointer_UTF8("\xe9\x87\x8d\xe7\xbd\xae");  // 重置
    stringsZhTw[StringKey::ButtonNextRound] = juce::CharPointer_UTF8("\xe4\xb8\x8b\xe4\xb8\x80\xe5\x9b\x9e\xe5\x90\x88");  // 下一回合
    stringsZhTw[StringKey::ButtonSubmit] = juce::CharPointer_UTF8("\xe6\x8f\x90\xe4\xba\xa4");  // 提交

    // Control Panel - Labels
    stringsZhTw[StringKey::LabelRounds] = juce::CharPointer_UTF8("\xe5\x9b\x9e\xe5\x90\x88\xe6\x95\xb8");  // 回合數
    stringsZhTw[StringKey::LabelAutoGain] = juce::CharPointer_UTF8("\xe8\x87\xaa\xe5\x8b\x95\xe5\xa2\x9e\xe7\x9b\x8a");  // 自動增益
    stringsZhTw[StringKey::LabelQuestions] = juce::CharPointer_UTF8("\xe5\x95\x8f\xe9\xa1\x8c\xe6\x95\xb8");  // 問題數
    stringsZhTw[StringKey::LabelCards] = juce::CharPointer_UTF8("\xe5\x8d\xa1\xe7\x89\x87\xe6\x95\xb8");  // 卡片數

    // Control Panel - Status
    stringsZhTw[StringKey::StatusReady] = juce::CharPointer_UTF8("\xe5\xb0\xb1\xe7\xb7\x92");  // 就緒
    stringsZhTw[StringKey::StatusShuffled] = juce::CharPointer_UTF8("\xe5\xb7\xb2\xe6\xb4\x97\xe7\x89\x8c");  // 已洗牌
    stringsZhTw[StringKey::StatusRevealed] = juce::CharPointer_UTF8("\xe5\xb7\xb2\xe6\x8f\xad\xe7\xa4\xba");  // 已揭示
    stringsZhTw[StringKey::StatusRound] = juce::CharPointer_UTF8("\xe7\xac\xac");  // 第
    stringsZhTw[StringKey::StatusOf] = juce::CharPointer_UTF8("\xe5\x9b\x9e\xe5\x90\x88\xef\xbc\x8c\xe5\x85\xb1");  // 回合，共
    stringsZhTw[StringKey::StatusDetecting] = juce::CharPointer_UTF8("\xe6\xaa\xa2\xe6\xb8\xac\xe4\xb8\xad...");  // 檢測中...
    stringsZhTw[StringKey::StatusWaiting] = juce::CharPointer_UTF8("\xe7\xad\x89\xe5\xbe\x85\xe4\xb8\xad...");  // 等待中...

    // Results Panel
    stringsZhTw[StringKey::ResultsTitle] = juce::CharPointer_UTF8("\xe7\xb5\x90\xe6\x9e\x9c");  // 結果
    stringsZhTw[StringKey::ResultsEmpty] = juce::CharPointer_UTF8("\xe5\xb0\x9a\xe7\x84\xa1\xe7\xb5\x90\xe6\x9e\x9c");  // 尚無結果
    stringsZhTw[StringKey::ResultsTrack] = juce::CharPointer_UTF8("\xe9\x9f\xb3\xe8\xbb\x8c");  // 音軌
    stringsZhTw[StringKey::ResultsScore] = juce::CharPointer_UTF8("\xe5\x88\x86\xe6\x95\xb8");  // 分數
    stringsZhTw[StringKey::ResultsAverage] = juce::CharPointer_UTF8("\xe5\xb9\xb3\xe5\x9d\x87");  // 平均
    stringsZhTw[StringKey::ResultsCorrect] = juce::CharPointer_UTF8("\xe6\xad\xa3\xe7\xa2\xba");  // 正確
    stringsZhTw[StringKey::ResultsIncorrect] = juce::CharPointer_UTF8("\xe9\x8c\xaf\xe8\xaa\xa4");  // 錯誤
    stringsZhTw[StringKey::ResultsTotal] = juce::CharPointer_UTF8("\xe7\xb8\xbd\xe8\xa8\x88");  // 總計

    // Question Banner (Q&A mode)
    stringsZhTw[StringKey::QAQuestion] = juce::CharPointer_UTF8("\xe5\x95\x8f\xe9\xa1\x8c");  // 問題
    stringsZhTw[StringKey::QASelectCard] = juce::CharPointer_UTF8("\xe8\xab\x8b\xe9\x81\xb8\xe6\x93\x87\xe4\xb8\x80\xe5\xbc\xb5\xe5\x8d\xa1\xe7\x89\x87");  // 請選擇一張卡片
    stringsZhTw[StringKey::QAWhichBetter] = juce::CharPointer_UTF8("\xe5\x93\xaa\xe5\x80\x8b\xe8\x81\xbd\xe8\xb5\xb7\xe4\xbe\x86\xe6\x9b\xb4\xe5\xa5\xbd\xef\xbc\x9f");  // 哪個聽起來更好？
    stringsZhTw[StringKey::QAPreference] = juce::CharPointer_UTF8("\xe4\xbd\xa0\xe7\x9a\x84\xe5\x81\x8f\xe5\xa5\xbd");  // 你的偏好

    // Poker Card
    stringsZhTw[StringKey::CardDropHere] = juce::CharPointer_UTF8("\xe6\x8b\x96\xe6\x9b\xb3\xe9\x9f\xb3\xe8\xa8\x8a\xe6\xaa\x94\xe6\xa1\x88\xe8\x87\xb3\xe6\xad\xa4");  // 拖曳音訊檔案至此
    stringsZhTw[StringKey::CardDragFile] = juce::CharPointer_UTF8("\xe6\x8b\x96\xe6\x9b\xb3\xe9\x9f\xb3\xe8\xa8\x8a");  // 拖曳音訊
    stringsZhTw[StringKey::CardTrack] = juce::CharPointer_UTF8("\xe9\x9f\xb3\xe8\xbb\x8c");  // 音軌

    // Card Count Control
    stringsZhTw[StringKey::CardCountLabel] = juce::CharPointer_UTF8("\xe5\x8d\xa1\xe7\x89\x87\xe6\x95\xb8\xe9\x87\x8f");  // 卡片數量

    // Settings Panel
    stringsZhTw[StringKey::SettingsTitle] = juce::CharPointer_UTF8("\xe8\xa8\xad\xe5\xae\x9a");  // 設定
    stringsZhTw[StringKey::SettingsLanguage] = juce::CharPointer_UTF8("\xe8\xaa\x9e\xe8\xa8\x80");  // 語言
    stringsZhTw[StringKey::SettingsTheme] = juce::CharPointer_UTF8("\xe4\xb8\xbb\xe9\xa1\x8c");  // 主題
    stringsZhTw[StringKey::SettingsThemeLight] = juce::CharPointer_UTF8("\xe6\xb7\xba\xe8\x89\xb2");  // 淺色
    stringsZhTw[StringKey::SettingsThemeDark] = juce::CharPointer_UTF8("\xe6\xb7\xb1\xe8\x89\xb2");  // 深色
    stringsZhTw[StringKey::SettingsClose] = juce::CharPointer_UTF8("\xe9\x97\x9c\xe9\x96\x89");  // 關閉

    // About Dialog
    stringsZhTw[StringKey::AboutTitle] = juce::CharPointer_UTF8("\xe9\x97\x9c\xe6\x96\xbc");  // 關於
    stringsZhTw[StringKey::AboutFreeSoftware] = juce::CharPointer_UTF8("\xe5\x85\x8d\xe8\xb2\xbb\xe8\xbb\x9f\xe9\xab\x94");  // 免費軟體
    stringsZhTw[StringKey::AboutNoLicense] = juce::CharPointer_UTF8("\xe7\x84\xa1\xe9\x9c\x80\xe6\x8e\x88\xe6\xac\x8a\xe9\x87\x91\xe9\x91\xb0");  // 無需授權金鑰
    stringsZhTw[StringKey::AboutVersion] = juce::CharPointer_UTF8("\xe7\x89\x88\xe6\x9c\xac\xef\xbc\x9a");  // 版本：
    stringsZhTw[StringKey::AboutDeveloper] = juce::CharPointer_UTF8("\xe9\x96\x8b\xe7\x99\xbc\xe8\x80\x85\xef\xbc\x9a");  // 開發者：
    stringsZhTw[StringKey::AboutVisitWebsite] = juce::CharPointer_UTF8("\xe9\x80\xa0\xe8\xa8\xaa\xe5\xae\x98\xe6\x96\xb9\xe7\xb6\xb2\xe7\xab\x99");  // 造訪官方網站

    // Quick Start Guide
    stringsZhTw[StringKey::GuideTitle] = juce::CharPointer_UTF8("\xe5\xbf\xab\xe9\x80\x9f\xe9\x96\x8b\xe5\xa7\x8b\xe6\x8c\x87\xe5\x8d\x97");  // 快速開始指南
    stringsZhTw[StringKey::GuideGotIt] = juce::CharPointer_UTF8("\xe6\x88\x91\xe7\x9f\xa5\xe9\x81\x93\xe4\xba\x86");  // 我知道了
    stringsZhTw[StringKey::GuideDescription] = juce::CharPointer_UTF8("Blind Card \xe8\xae\x93\xe4\xbd\xa0\xe5\x9c\xa8\xe4\xb8\x8d\xe7\x9f\xa5\xe9\x81\x93\xe9\x9f\xb3\xe8\xbb\x8c\xe8\xba\xab\xe4\xbb\xbd\xe7\x9a\x84\xe6\x83\x85\xe6\xb3\x81\xe4\xb8\x8b\xe6\xaf\x94\xe8\xbc\x83\xe9\x9f\xb3\xe8\xa8\x8a\xef\xbc\x8c\xe6\xb6\x88\xe9\x99\xa4\xe6\xb1\xba\xe7\xad\x96\xe4\xb8\xad\xe7\x9a\x84\xe5\x81\x8f\xe8\xa6\x8b\xe3\x80\x82");  // Blind Card 讓你在不知道音軌身份的情況下比較音訊，消除決策中的偏見。

    // Steps (full detailed text)
    stringsZhTw[StringKey::GuideStep1Full] = juce::CharPointer_UTF8("\xe5\x9c\xa8\xe6\xaf\x8f\xe5\x80\x8b\xe8\xa6\x81\xe6\xaf\x94\xe8\xbc\x83\xe7\x9a\x84\xe9\x9f\xb3\xe8\xbb\x8c\xe4\xb8\x8a\xe6\x8f\x92\xe5\x85\xa5 Blind Card (2-8 \xe5\x80\x8b\xe9\x9f\xb3\xe8\xbb\x8c)");  // 在每個要比較的音軌上插入 Blind Card (2-8 個音軌)
    stringsZhTw[StringKey::GuideStep2Intro] = juce::CharPointer_UTF8("\xe9\x81\xb8\xe6\x93\x87\xe8\xa9\x95\xe5\x88\x86\xe6\xa8\xa1\xe5\xbc\x8f\xef\xbc\x9a");  // 選擇評分模式：
    stringsZhTw[StringKey::GuideStep3Full] = juce::CharPointer_UTF8("\xe9\xbb\x9e\xe6\x93\x8a\xe3\x80\x8c\xe6\xb4\x97\xe7\x89\x8c\xe3\x80\x8d\xe9\x9a\xa8\xe6\xa9\x9f\xe6\x8e\x92\xe5\x88\x97\xe4\xb8\xa6\xe9\x9a\xb1\xe8\x97\x8f\xe9\x9f\xb3\xe8\xbb\x8c\xe8\xba\xab\xe4\xbb\xbd");  // 點擊「洗牌」隨機排列並隱藏音軌身份
    stringsZhTw[StringKey::GuideStep4Full] = juce::CharPointer_UTF8("\xe9\xbb\x9e\xe6\x93\x8a\xe5\x8d\xa1\xe7\x89\x87\xe8\x81\x86\xe8\x81\xbd\xe4\xb8\xa6\xe8\xa9\x95\xe5\x88\x86\xef\xbc\x88\xe6\x88\x96\xe4\xbd\xbf\xe7\x94\xa8\xe6\x96\xb9\xe5\x90\x91\xe9\x8d\xb5\xef\xbc\x89");  // 點擊卡片聆聽並評分（或使用方向鍵）
    stringsZhTw[StringKey::GuideStep5Full] = juce::CharPointer_UTF8("\xe9\xbb\x9e\xe6\x93\x8a\xe3\x80\x8c\xe6\x8f\xad\xe7\xa4\xba\xe3\x80\x8d\xe6\x9f\xa5\xe7\x9c\x8b\xe7\xb5\x90\xe6\x9e\x9c\xe5\x92\x8c\xe5\xaf\xa6\xe9\x9a\x9b\xe9\x9f\xb3\xe8\xbb\x8c\xe5\x90\x8d\xe7\xa8\xb1");  // 點擊「揭示」查看結果和實際音軌名稱
    stringsZhTw[StringKey::GuideStep6Full] = juce::CharPointer_UTF8("\xe9\xbb\x9e\xe6\x93\x8a\xe3\x80\x8c\xe9\x87\x8d\xe7\xbd\xae\xe3\x80\x8d\xe9\x96\x8b\xe5\xa7\x8b\xe6\x96\xb0\xe7\x9a\x84\xe6\xb8\xac\xe8\xa9\xa6");  // 點擊「重置」開始新的測試

    // Mode descriptions in guide
    stringsZhTw[StringKey::GuideModeStarsDesc] = juce::CharPointer_UTF8("\xe7\x82\xba\xe6\xaf\x8f\xe5\x80\x8b\xe9\x9f\xb3\xe8\xbb\x8c\xe8\xa9\x95\xe5\x88\x86 1-5 \xe9\xa1\x86\xe6\x98\x9f");  // 為每個音軌評分 1-5 顆星
    stringsZhTw[StringKey::GuideModeGuessDesc] = juce::CharPointer_UTF8("\xe8\xad\x98\xe5\x88\xa5\xe6\xaf\x8f\xe5\xbc\xb5\xe5\x8d\xa1\xe7\x89\x87\xe4\xb8\x8a\xe6\x98\xaf\xe5\x93\xaa\xe5\x80\x8b\xe6\x8f\x92\xe4\xbb\xb6/\xe6\xb7\xb7\xe9\x9f\xb3");  // 識別每張卡片上是哪個插件/混音
    stringsZhTw[StringKey::GuideModeQADesc] = juce::CharPointer_UTF8("\xe6\xa0\xb9\xe6\x93\x9a\xe6\x8f\x90\xe7\xa4\xba\xe6\x89\xbe\xe5\x87\xba\xe7\x89\xb9\xe5\xae\x9a\xe9\x9f\xb3\xe8\xbb\x8c");  // 根據提示找出特定音軌

    // Tip box
    stringsZhTw[StringKey::GuideTip] = juce::CharPointer_UTF8("\xe6\x8f\x90\xe7\xa4\xba");  // 提示
    stringsZhTw[StringKey::GuideTipText1] = juce::CharPointer_UTF8("\xe5\x95\x9f\xe7\x94\xa8\xe3\x80\x8c\xe8\x87\xaa\xe5\x8b\x95\xe5\xa2\x9e\xe7\x9b\x8a\xe3\x80\x8d\xe4\xbe\x86\xe9\x9f\xbf\xe5\xba\xa6\xe5\x8c\xb9\xe9\x85\x8d\xe6\x89\x80\xe6\x9c\x89\xe9\x9f\xb3\xe8\xbb\x8c\xef\xbc\x8c");  // 啟用「自動增益」來響度匹配所有音軌，
    stringsZhTw[StringKey::GuideTipText2] = juce::CharPointer_UTF8("\xe7\xa2\xba\xe4\xbf\x9d\xe5\x85\xac\xe5\xb9\xb3\xe6\xaf\x94\xe8\xbc\x83\xef\xbc\x8c\xe4\xb8\x8d\xe5\x8f\x97\xe9\x9f\xb3\xe9\x87\x8f\xe5\x81\x8f\xe8\xa6\x8b\xe5\xbd\xb1\xe9\x9f\xbf\xe3\x80\x82");  // 確保公平比較，不受音量偏見影響。

    // Keyboard shortcuts
    stringsZhTw[StringKey::GuideKeyboardShortcuts] = juce::CharPointer_UTF8("\xe9\x8d\xb5\xe7\x9b\xa4\xe5\xbf\xab\xe6\x8d\xb7\xe9\x8d\xb5");  // 鍵盤快捷鍵
    stringsZhTw[StringKey::GuideShortcutSelectCard] = juce::CharPointer_UTF8("\xe7\x9b\xb4\xe6\x8e\xa5\xe9\x81\xb8\xe6\x93\x87\xe5\x8d\xa1\xe7\x89\x87");  // 直接選擇卡片
    stringsZhTw[StringKey::GuideShortcutNextPrev] = juce::CharPointer_UTF8("\xe4\xb8\x8b\xe4\xb8\x80\xe5\xbc\xb5 / \xe4\xb8\x8a\xe4\xb8\x80\xe5\xbc\xb5\xe5\x8d\xa1\xe7\x89\x87");  // 下一張 / 上一張卡片
    stringsZhTw[StringKey::GuideShortcutBrackets] = juce::CharPointer_UTF8("\xe4\xb8\x8a\xe4\xb8\x80\xe5\xbc\xb5 / \xe4\xb8\x8b\xe4\xb8\x80\xe5\xbc\xb5\xe5\x8d\xa1\xe7\x89\x87");  // 上一張 / 下一張卡片
    stringsZhTw[StringKey::GuideShortcutArrows] = juce::CharPointer_UTF8("\xe5\xb0\x8e\xe8\x88\xaa\xe5\x8d\xa1\xe7\x89\x87\xef\xbc\x88\xe5\x9c\xa8 Logic Pro \xe4\xb8\xad\xe5\x8f\xaf\xe8\x83\xbd\xe4\xb8\x8d\xe4\xbd\x9c\xe7\x94\xa8\xef\xbc\x89");  // 導航卡片（在 Logic Pro 中可能不作用）

    // Dialogs
    stringsZhTw[StringKey::DialogSavePreset] = juce::CharPointer_UTF8("\xe5\x84\xb2\xe5\xad\x98\xe9\xa0\x90\xe8\xa8\xad");  // 儲存預設
    stringsZhTw[StringKey::DialogEnterName] = juce::CharPointer_UTF8("\xe8\xab\x8b\xe8\xbc\xb8\xe5\x85\xa5\xe9\xa0\x90\xe8\xa8\xad\xe5\x90\x8d\xe7\xa8\xb1\xef\xbc\x9a");  // 請輸入預設名稱：
    stringsZhTw[StringKey::DialogSave] = juce::CharPointer_UTF8("\xe5\x84\xb2\xe5\xad\x98");  // 儲存
    stringsZhTw[StringKey::DialogCancel] = juce::CharPointer_UTF8("\xe5\x8f\x96\xe6\xb6\x88");  // 取消
    stringsZhTw[StringKey::DialogDelete] = juce::CharPointer_UTF8("\xe5\x88\xaa\xe9\x99\xa4");  // 刪除
    stringsZhTw[StringKey::DialogConfirmDelete] = juce::CharPointer_UTF8("\xe7\xa2\xba\xe5\xae\x9a\xe8\xa6\x81\xe5\x88\xaa\xe9\x99\xa4\xe6\xad\xa4\xe9\xa0\x90\xe8\xa8\xad\xe5\x97\x8e\xef\xbc\x9f");  // 確定要刪除此預設嗎？
    stringsZhTw[StringKey::DialogYes] = juce::CharPointer_UTF8("\xe6\x98\xaf");  // 是
    stringsZhTw[StringKey::DialogNo] = juce::CharPointer_UTF8("\xe5\x90\xa6");  // 否

    // Import
    stringsZhTw[StringKey::ImportFiles] = juce::CharPointer_UTF8("\xe5\x8c\xaf\xe5\x85\xa5\xe6\xaa\x94\xe6\xa1\x88");  // 匯入檔案
    stringsZhTw[StringKey::ImportSuccess] = juce::CharPointer_UTF8("\xe5\x8c\xaf\xe5\x85\xa5\xe6\x88\x90\xe5\x8a\x9f");  // 匯入成功
    stringsZhTw[StringKey::ImportFailed] = juce::CharPointer_UTF8("\xe5\x8c\xaf\xe5\x85\xa5\xe5\xa4\xb1\xe6\x95\x97");  // 匯入失敗
    stringsZhTw[StringKey::ImportFilesLoaded] = juce::CharPointer_UTF8("\xe5\x80\x8b\xe6\xaa\x94\xe6\xa1\x88\xe5\xb7\xb2\xe8\xbc\x89\xe5\x85\xa5");  // 個檔案已載入

    // Transport Bar
    stringsZhTw[StringKey::TransportPlay] = juce::CharPointer_UTF8("\xe6\x92\xad\xe6\x94\xbe");  // 播放
    stringsZhTw[StringKey::TransportPause] = juce::CharPointer_UTF8("\xe6\x9a\xab\xe5\x81\x9c");  // 暫停
    stringsZhTw[StringKey::TransportStop] = juce::CharPointer_UTF8("\xe5\x81\x9c\xe6\xad\xa2");  // 停止

    // Presets
    stringsZhTw[StringKey::PresetDefault] = juce::CharPointer_UTF8("\xe9\xa0\x90\xe8\xa8\xad");  // 預設
    stringsZhTw[StringKey::PresetSave] = juce::CharPointer_UTF8("\xe5\x84\xb2\xe5\xad\x98");  // 儲存
    stringsZhTw[StringKey::PresetDelete] = juce::CharPointer_UTF8("\xe5\x88\xaa\xe9\x99\xa4");  // 刪除
    stringsZhTw[StringKey::PresetSelect] = juce::CharPointer_UTF8("\xe9\x81\xb8\xe6\x93\x87\xe9\xa0\x90\xe8\xa8\xad");  // 選擇預設

    // Star Rating
    stringsZhTw[StringKey::RatingLabel] = juce::CharPointer_UTF8("\xe8\xa9\x95\xe5\x88\x86");  // 評分

    // Guess Dropdown
    stringsZhTw[StringKey::GuessSelect] = juce::CharPointer_UTF8("\xe9\x81\xb8\xe6\x93\x87\xe9\x9f\xb3\xe8\xbb\x8c...");  // 選擇音軌...
    stringsZhTw[StringKey::GuessNone] = juce::CharPointer_UTF8("\xe7\x84\xa1");  // 無

    //==========================================================================
    // JAPANESE (日本語)
    //==========================================================================

    // Header Bar
    stringsJa[StringKey::NowPlaying] = juce::CharPointer_UTF8("\xe5\x86\x8d\xe7\x94\x9f\xe4\xb8\xad\xef\xbc\x9a");  // 再生中：
    stringsJa[StringKey::NoFileLoaded] = juce::CharPointer_UTF8("\xe3\x83\x95\xe3\x82\xa1\xe3\x82\xa4\xe3\x83\xab\xe6\x9c\xaa\xe8\xaa\xad\xe8\xbe\xbc");  // ファイル未読込

    // Mode Selector
    stringsJa[StringKey::ModeStars] = juce::CharPointer_UTF8("\xe6\x98\x9f\xe8\xa9\x95\xe4\xbe\xa1");  // 星評価
    stringsJa[StringKey::ModeGuess] = juce::CharPointer_UTF8("\xe6\x8e\xa8\xe6\xb8\xac");  // 推測
    stringsJa[StringKey::ModeQA] = juce::CharPointer_UTF8("\xe8\xb3\xaa\xe5\x95\x8f");  // 質問

    // Control Panel - Buttons
    stringsJa[StringKey::ButtonShuffle] = juce::CharPointer_UTF8("\xe3\x82\xb7\xe3\x83\xa3\xe3\x83\x83\xe3\x83\x95\xe3\x83\xab");  // シャッフル
    stringsJa[StringKey::ButtonReveal] = juce::CharPointer_UTF8("\xe5\x85\xac\xe9\x96\x8b");  // 公開
    stringsJa[StringKey::ButtonReset] = juce::CharPointer_UTF8("\xe3\x83\xaa\xe3\x82\xbb\xe3\x83\x83\xe3\x83\x88");  // リセット
    stringsJa[StringKey::ButtonNextRound] = juce::CharPointer_UTF8("\xe6\xac\xa1\xe3\x81\xae\xe3\x83\xa9\xe3\x82\xa6\xe3\x83\xb3\xe3\x83\x89");  // 次のラウンド
    stringsJa[StringKey::ButtonSubmit] = juce::CharPointer_UTF8("\xe9\x80\x81\xe4\xbf\xa1");  // 送信

    // Control Panel - Labels
    stringsJa[StringKey::LabelRounds] = juce::CharPointer_UTF8("\xe3\x83\xa9\xe3\x82\xa6\xe3\x83\xb3\xe3\x83\x89\xe6\x95\xb0");  // ラウンド数
    stringsJa[StringKey::LabelAutoGain] = juce::CharPointer_UTF8("\xe8\x87\xaa\xe5\x8b\x95\xe3\x82\xb2\xe3\x82\xa4\xe3\x83\xb3");  // 自動ゲイン
    stringsJa[StringKey::LabelQuestions] = juce::CharPointer_UTF8("\xe8\xb3\xaa\xe5\x95\x8f\xe6\x95\xb0");  // 質問数
    stringsJa[StringKey::LabelCards] = juce::CharPointer_UTF8("\xe3\x82\xab\xe3\x83\xbc\xe3\x83\x89\xe6\x95\xb0");  // カード数

    // Control Panel - Status
    stringsJa[StringKey::StatusReady] = juce::CharPointer_UTF8("\xe6\xba\x96\xe5\x82\x99\xe5\xae\x8c\xe4\xba\x86");  // 準備完了
    stringsJa[StringKey::StatusShuffled] = juce::CharPointer_UTF8("\xe3\x82\xb7\xe3\x83\xa3\xe3\x83\x83\xe3\x83\x95\xe3\x83\xab\xe6\xb8\x88");  // シャッフル済
    stringsJa[StringKey::StatusRevealed] = juce::CharPointer_UTF8("\xe5\x85\xac\xe9\x96\x8b\xe6\xb8\x88");  // 公開済
    stringsJa[StringKey::StatusRound] = juce::CharPointer_UTF8("\xe3\x83\xa9\xe3\x82\xa6\xe3\x83\xb3\xe3\x83\x89");  // ラウンド
    stringsJa[StringKey::StatusOf] = juce::CharPointer_UTF8("/");  // /
    stringsJa[StringKey::StatusDetecting] = juce::CharPointer_UTF8("\xe6\xa4\x9c\xe5\x87\xba\xe4\xb8\xad...");  // 検出中...
    stringsJa[StringKey::StatusWaiting] = juce::CharPointer_UTF8("\xe5\xbe\x85\xe6\xa9\x9f\xe4\xb8\xad...");  // 待機中...

    // Results Panel
    stringsJa[StringKey::ResultsTitle] = juce::CharPointer_UTF8("\xe7\xb5\x90\xe6\x9e\x9c");  // 結果
    stringsJa[StringKey::ResultsEmpty] = juce::CharPointer_UTF8("\xe7\xb5\x90\xe6\x9e\x9c\xe3\x81\xaa\xe3\x81\x97");  // 結果なし
    stringsJa[StringKey::ResultsTrack] = juce::CharPointer_UTF8("\xe3\x83\x88\xe3\x83\xa9\xe3\x83\x83\xe3\x82\xaf");  // トラック
    stringsJa[StringKey::ResultsScore] = juce::CharPointer_UTF8("\xe3\x82\xb9\xe3\x82\xb3\xe3\x82\xa2");  // スコア
    stringsJa[StringKey::ResultsAverage] = juce::CharPointer_UTF8("\xe5\xb9\xb3\xe5\x9d\x87");  // 平均
    stringsJa[StringKey::ResultsCorrect] = juce::CharPointer_UTF8("\xe6\xad\xa3\xe8\xa7\xa3");  // 正解
    stringsJa[StringKey::ResultsIncorrect] = juce::CharPointer_UTF8("\xe4\xb8\x8d\xe6\xad\xa3\xe8\xa7\xa3");  // 不正解
    stringsJa[StringKey::ResultsTotal] = juce::CharPointer_UTF8("\xe5\x90\x88\xe8\xa8\x88");  // 合計

    // Question Banner (Q&A mode)
    stringsJa[StringKey::QAQuestion] = juce::CharPointer_UTF8("\xe8\xb3\xaa\xe5\x95\x8f");  // 質問
    stringsJa[StringKey::QASelectCard] = juce::CharPointer_UTF8("\xe3\x82\xab\xe3\x83\xbc\xe3\x83\x89\xe3\x82\x92\xe9\x81\xb8\xe6\x8a\x9e");  // カードを選択
    stringsJa[StringKey::QAWhichBetter] = juce::CharPointer_UTF8("\xe3\x81\xa9\xe3\x81\xa1\xe3\x82\x89\xe3\x81\x8c\xe8\x89\xaf\xe3\x81\x84\xe3\x81\xa7\xe3\x81\x99\xe3\x81\x8b\xef\xbc\x9f");  // どちらが良いですか？
    stringsJa[StringKey::QAPreference] = juce::CharPointer_UTF8("\xe3\x81\x82\xe3\x81\xaa\xe3\x81\x9f\xe3\x81\xae\xe9\x81\xb8\xe6\x8a\x9e");  // あなたの選択

    // Poker Card
    stringsJa[StringKey::CardDropHere] = juce::CharPointer_UTF8("\xe3\x81\x93\xe3\x81\x93\xe3\x81\xab\xe3\x83\x95\xe3\x82\xa1\xe3\x82\xa4\xe3\x83\xab\xe3\x82\x92\xe3\x83\x89\xe3\x83\xad\xe3\x83\x83\xe3\x83\x97");  // ここにファイルをドロップ
    stringsJa[StringKey::CardDragFile] = juce::CharPointer_UTF8("\xe3\x83\x89\xe3\x83\xa9\xe3\x83\x83\xe3\x82\xb0\xef\xbc\x86\xe3\x83\x89\xe3\x83\xad\xe3\x83\x83\xe3\x83\x97");  // ドラッグ＆ドロップ
    stringsJa[StringKey::CardTrack] = juce::CharPointer_UTF8("\xe3\x83\x88\xe3\x83\xa9\xe3\x83\x83\xe3\x82\xaf");  // トラック

    // Card Count Control
    stringsJa[StringKey::CardCountLabel] = juce::CharPointer_UTF8("\xe3\x82\xab\xe3\x83\xbc\xe3\x83\x89\xe6\x95\xb0");  // カード数

    // Settings Panel
    stringsJa[StringKey::SettingsTitle] = juce::CharPointer_UTF8("\xe8\xa8\xad\xe5\xae\x9a");  // 設定
    stringsJa[StringKey::SettingsLanguage] = juce::CharPointer_UTF8("\xe8\xa8\x80\xe8\xaa\x9e");  // 言語
    stringsJa[StringKey::SettingsTheme] = juce::CharPointer_UTF8("\xe3\x83\x86\xe3\x83\xbc\xe3\x83\x9e");  // テーマ
    stringsJa[StringKey::SettingsThemeLight] = juce::CharPointer_UTF8("\xe3\x83\xa9\xe3\x82\xa4\xe3\x83\x88");  // ライト
    stringsJa[StringKey::SettingsThemeDark] = juce::CharPointer_UTF8("\xe3\x83\x80\xe3\x83\xbc\xe3\x82\xaf");  // ダーク
    stringsJa[StringKey::SettingsClose] = juce::CharPointer_UTF8("\xe9\x96\x89\xe3\x81\x98\xe3\x82\x8b");  // 閉じる

    // About Dialog
    stringsJa[StringKey::AboutTitle] = juce::CharPointer_UTF8("\xe3\x81\x93\xe3\x81\xae\xe3\x82\xa2\xe3\x83\x97\xe3\x83\xaa\xe3\x81\xab\xe3\x81\xa4\xe3\x81\x84\xe3\x81\xa6");  // このアプリについて
    stringsJa[StringKey::AboutFreeSoftware] = juce::CharPointer_UTF8("\xe3\x83\x95\xe3\x83\xaa\xe3\x83\xbc\xe3\x82\xbd\xe3\x83\x95\xe3\x83\x88\xe3\x82\xa6\xe3\x82\xa7\xe3\x82\xa2");  // フリーソフトウェア
    stringsJa[StringKey::AboutNoLicense] = juce::CharPointer_UTF8("\xe3\x83\xa9\xe3\x82\xa4\xe3\x82\xbb\xe3\x83\xb3\xe3\x82\xb9\xe3\x82\xad\xe3\x83\xbc\xe4\xb8\x8d\xe8\xa6\x81");  // ライセンスキー不要
    stringsJa[StringKey::AboutVersion] = juce::CharPointer_UTF8("\xe3\x83\x90\xe3\x83\xbc\xe3\x82\xb8\xe3\x83\xa7\xe3\x83\xb3\xef\xbc\x9a");  // バージョン：
    stringsJa[StringKey::AboutDeveloper] = juce::CharPointer_UTF8("\xe9\x96\x8b\xe7\x99\xba\xe8\x80\x85\xef\xbc\x9a");  // 開発者：
    stringsJa[StringKey::AboutVisitWebsite] = juce::CharPointer_UTF8("\xe5\x85\xac\xe5\xbc\x8f\xe3\x82\xb5\xe3\x82\xa4\xe3\x83\x88\xe3\x82\x92\xe8\xa8\xaa\xe5\x95\x8f");  // 公式サイトを訪問

    // Quick Start Guide
    stringsJa[StringKey::GuideTitle] = juce::CharPointer_UTF8("\xe3\x82\xaf\xe3\x82\xa4\xe3\x83\x83\xe3\x82\xaf\xe3\x82\xb9\xe3\x82\xbf\xe3\x83\xbc\xe3\x83\x88");  // クイックスタート
    stringsJa[StringKey::GuideGotIt] = juce::CharPointer_UTF8("\xe4\xba\x86\xe8\xa7\xa3");  // 了解
    stringsJa[StringKey::GuideDescription] = juce::CharPointer_UTF8("Blind Card \xe3\x81\xa7\xe3\x81\xa9\xe3\x81\xae\xe3\x83\x88\xe3\x83\xa9\xe3\x83\x83\xe3\x82\xaf\xe3\x81\x8b\xe5\x88\x86\xe3\x81\x8b\xe3\x82\x89\xe3\x81\xaa\xe3\x81\x84\xe7\x8a\xb6\xe6\x85\x8b\xe3\x81\xa7\xe3\x82\xaa\xe3\x83\xbc\xe3\x83\x87\xe3\x82\xa3\xe3\x82\xaa\xe3\x82\x92\xe6\xaf\x94\xe8\xbc\x83\xe3\x81\x97\xe3\x80\x81\xe6\x84\x8f\xe6\x80\x9d\xe6\xb1\xba\xe5\xae\x9a\xe3\x81\x8b\xe3\x82\x89\xe3\x83\x90\xe3\x82\xa4\xe3\x82\xa2\xe3\x82\xb9\xe3\x82\x92\xe6\x8e\x92\xe9\x99\xa4\xe3\x81\x97\xe3\x81\xbe\xe3\x81\x99\xe3\x80\x82");  // Blind Card でどのトラックか分からない状態でオーディオを比較し、意思決定からバイアスを排除します。

    // Steps (full detailed text)
    stringsJa[StringKey::GuideStep1Full] = juce::CharPointer_UTF8("\xe6\xaf\x94\xe8\xbc\x83\xe3\x81\x97\xe3\x81\x9f\xe3\x81\x84\xe5\x90\x84\xe3\x83\x88\xe3\x83\xa9\xe3\x83\x83\xe3\x82\xaf\xe3\x81\xab Blind Card \xe3\x82\x92\xe6\x8c\xbf\xe5\x85\xa5 (2-8 \xe3\x83\x88\xe3\x83\xa9\xe3\x83\x83\xe3\x82\xaf)");  // 比較したい各トラックに Blind Card を挿入 (2-8 トラック)
    stringsJa[StringKey::GuideStep2Intro] = juce::CharPointer_UTF8("\xe8\xa9\x95\xe4\xbe\xa1\xe3\x83\xa2\xe3\x83\xbc\xe3\x83\x89\xe3\x82\x92\xe9\x81\xb8\xe6\x8a\x9e\xef\xbc\x9a");  // 評価モードを選択：
    stringsJa[StringKey::GuideStep3Full] = juce::CharPointer_UTF8("\xe3\x80\x8c\xe3\x82\xb7\xe3\x83\xa3\xe3\x83\x83\xe3\x83\x95\xe3\x83\xab\xe3\x80\x8d\xe3\x82\x92\xe3\x82\xaf\xe3\x83\xaa\xe3\x83\x83\xe3\x82\xaf\xe3\x81\x97\xe3\x81\xa6\xe3\x83\xa9\xe3\x83\xb3\xe3\x83\x80\xe3\x83\xa0\xe5\x8c\x96\xe3\x81\x97\xe3\x80\x81\xe3\x83\x88\xe3\x83\xa9\xe3\x83\x83\xe3\x82\xaf\xe5\x90\x8d\xe3\x82\x92\xe9\x9a\xa0\xe3\x81\x99");  // 「シャッフル」をクリックしてランダム化し、トラック名を隠す
    stringsJa[StringKey::GuideStep4Full] = juce::CharPointer_UTF8("\xe3\x82\xab\xe3\x83\xbc\xe3\x83\x89\xe3\x82\x92\xe3\x82\xaf\xe3\x83\xaa\xe3\x83\x83\xe3\x82\xaf\xe3\x81\x97\xe3\x81\xa6\xe8\x81\xb4\xe3\x81\x84\xe3\x81\xa6\xe8\xa9\x95\xe4\xbe\xa1\xef\xbc\x88\xe3\x81\xbe\xe3\x81\x9f\xe3\x81\xaf\xe7\x9f\xa2\xe5\x8d\xb0\xe3\x82\xad\xe3\x83\xbc\xe3\x82\x92\xe4\xbd\xbf\xe7\x94\xa8\xef\xbc\x89");  // カードをクリックして聴いて評価（または矢印キーを使用）
    stringsJa[StringKey::GuideStep5Full] = juce::CharPointer_UTF8("\xe3\x80\x8c\xe5\x85\xac\xe9\x96\x8b\xe3\x80\x8d\xe3\x82\x92\xe3\x82\xaf\xe3\x83\xaa\xe3\x83\x83\xe3\x82\xaf\xe3\x81\x97\xe3\x81\xa6\xe7\xb5\x90\xe6\x9e\x9c\xe3\x81\xa8\xe5\xae\x9f\xe9\x9a\x9b\xe3\x81\xae\xe3\x83\x88\xe3\x83\xa9\xe3\x83\x83\xe3\x82\xaf\xe5\x90\x8d\xe3\x82\x92\xe7\xa2\xba\xe8\xaa\x8d");  // 「公開」をクリックして結果と実際のトラック名を確認
    stringsJa[StringKey::GuideStep6Full] = juce::CharPointer_UTF8("\xe3\x80\x8c\xe3\x83\xaa\xe3\x82\xbb\xe3\x83\x83\xe3\x83\x88\xe3\x80\x8d\xe3\x82\x92\xe3\x82\xaf\xe3\x83\xaa\xe3\x83\x83\xe3\x82\xaf\xe3\x81\x97\xe3\x81\xa6\xe6\x96\xb0\xe3\x81\x97\xe3\x81\x84\xe3\x83\x86\xe3\x82\xb9\xe3\x83\x88\xe3\x82\x92\xe9\x96\x8b\xe5\xa7\x8b");  // 「リセット」をクリックして新しいテストを開始

    // Mode descriptions in guide
    stringsJa[StringKey::GuideModeStarsDesc] = juce::CharPointer_UTF8("\xe5\x90\x84\xe3\x83\x88\xe3\x83\xa9\xe3\x83\x83\xe3\x82\xaf\xe3\x82\x92 1-5 \xe6\x98\x9f\xe3\x81\xa7\xe8\xa9\x95\xe4\xbe\xa1");  // 各トラックを 1-5 星で評価
    stringsJa[StringKey::GuideModeGuessDesc] = juce::CharPointer_UTF8("\xe5\x90\x84\xe3\x82\xab\xe3\x83\xbc\xe3\x83\x89\xe3\x81\xae\xe3\x83\x97\xe3\x83\xa9\xe3\x82\xb0\xe3\x82\xa4\xe3\x83\xb3/\xe3\x83\x9f\xe3\x83\x83\xe3\x82\xaf\xe3\x82\xb9\xe3\x82\x92\xe8\xad\x98\xe5\x88\xa5");  // 各カードのプラグイン/ミックスを識別
    stringsJa[StringKey::GuideModeQADesc] = juce::CharPointer_UTF8("\xe3\x83\x97\xe3\x83\xad\xe3\x83\xb3\xe3\x83\x97\xe3\x83\x88\xe3\x81\xa7\xe6\x8c\x87\xe5\xae\x9a\xe3\x81\x95\xe3\x82\x8c\xe3\x81\x9f\xe3\x83\x88\xe3\x83\xa9\xe3\x83\x83\xe3\x82\xaf\xe3\x82\x92\xe8\xa6\x8b\xe3\x81\xa4\xe3\x81\x91\xe3\x82\x8b");  // プロンプトで指定されたトラックを見つける

    // Tip box
    stringsJa[StringKey::GuideTip] = juce::CharPointer_UTF8("\xe3\x83\x92\xe3\x83\xb3\xe3\x83\x88");  // ヒント
    stringsJa[StringKey::GuideTipText1] = juce::CharPointer_UTF8("\xe3\x80\x8c\xe8\x87\xaa\xe5\x8b\x95\xe3\x82\xb2\xe3\x82\xa4\xe3\x83\xb3\xe3\x80\x8d\xe3\x82\x92\xe6\x9c\x89\xe5\x8a\xb9\xe3\x81\xab\xe3\x81\x97\xe3\x81\xa6\xe5\x85\xa8\xe3\x83\x88\xe3\x83\xa9\xe3\x83\x83\xe3\x82\xaf\xe3\x81\xae\xe9\x9f\xb3\xe9\x87\x8f\xe3\x82\x92\xe3\x83\x9e\xe3\x83\x83\xe3\x83\x81\xe3\x81\x97\xe3\x80\x81");  // 「自動ゲイン」を有効にして全トラックの音量をマッチし、
    stringsJa[StringKey::GuideTipText2] = juce::CharPointer_UTF8("\xe9\x9f\xb3\xe9\x87\x8f\xe3\x83\x90\xe3\x82\xa4\xe3\x82\xa2\xe3\x82\xb9\xe3\x81\xaa\xe3\x81\x97\xe3\x81\xa7\xe5\x85\xac\xe5\xb9\xb3\xe3\x81\xab\xe6\xaf\x94\xe8\xbc\x83\xe3\x81\xa7\xe3\x81\x8d\xe3\x81\xbe\xe3\x81\x99\xe3\x80\x82");  // 音量バイアスなしで公平に比較できます。

    // Keyboard shortcuts
    stringsJa[StringKey::GuideKeyboardShortcuts] = juce::CharPointer_UTF8("\xe3\x82\xad\xe3\x83\xbc\xe3\x83\x9c\xe3\x83\xbc\xe3\x83\x89\xe3\x82\xb7\xe3\x83\xa7\xe3\x83\xbc\xe3\x83\x88\xe3\x82\xab\xe3\x83\x83\xe3\x83\x88");  // キーボードショートカット
    stringsJa[StringKey::GuideShortcutSelectCard] = juce::CharPointer_UTF8("\xe3\x82\xab\xe3\x83\xbc\xe3\x83\x89\xe3\x82\x92\xe7\x9b\xb4\xe6\x8e\xa5\xe9\x81\xb8\xe6\x8a\x9e");  // カードを直接選択
    stringsJa[StringKey::GuideShortcutNextPrev] = juce::CharPointer_UTF8("\xe6\xac\xa1 / \xe5\x89\x8d\xe3\x81\xae\xe3\x82\xab\xe3\x83\xbc\xe3\x83\x89");  // 次 / 前のカード
    stringsJa[StringKey::GuideShortcutBrackets] = juce::CharPointer_UTF8("\xe5\x89\x8d / \xe6\xac\xa1\xe3\x81\xae\xe3\x82\xab\xe3\x83\xbc\xe3\x83\x89");  // 前 / 次のカード
    stringsJa[StringKey::GuideShortcutArrows] = juce::CharPointer_UTF8("\xe3\x82\xab\xe3\x83\xbc\xe3\x83\x89\xe3\x82\x92\xe3\x83\x8a\xe3\x83\x93\xe3\x82\xb2\xe3\x83\xbc\xe3\x83\x88\xef\xbc\x88Logic Pro \xe3\x81\xa7\xe3\x81\xaf\xe5\x8b\x95\xe4\xbd\x9c\xe3\x81\x97\xe3\x81\xaa\xe3\x81\x84\xe5\xa0\xb4\xe5\x90\x88\xe3\x81\x8c\xe3\x81\x82\xe3\x82\x8a\xe3\x81\xbe\xe3\x81\x99\xef\xbc\x89");  // カードをナビゲート（Logic Pro では動作しない場合があります）

    // Dialogs
    stringsJa[StringKey::DialogSavePreset] = juce::CharPointer_UTF8("\xe3\x83\x97\xe3\x83\xaa\xe3\x82\xbb\xe3\x83\x83\xe3\x83\x88\xe3\x82\x92\xe4\xbf\x9d\xe5\xad\x98");  // プリセットを保存
    stringsJa[StringKey::DialogEnterName] = juce::CharPointer_UTF8("\xe3\x83\x97\xe3\x83\xaa\xe3\x82\xbb\xe3\x83\x83\xe3\x83\x88\xe5\x90\x8d\xe3\x82\x92\xe5\x85\xa5\xe5\x8a\x9b\xef\xbc\x9a");  // プリセット名を入力：
    stringsJa[StringKey::DialogSave] = juce::CharPointer_UTF8("\xe4\xbf\x9d\xe5\xad\x98");  // 保存
    stringsJa[StringKey::DialogCancel] = juce::CharPointer_UTF8("\xe3\x82\xad\xe3\x83\xa3\xe3\x83\xb3\xe3\x82\xbb\xe3\x83\xab");  // キャンセル
    stringsJa[StringKey::DialogDelete] = juce::CharPointer_UTF8("\xe5\x89\x8a\xe9\x99\xa4");  // 削除
    stringsJa[StringKey::DialogConfirmDelete] = juce::CharPointer_UTF8("\xe3\x81\x93\xe3\x81\xae\xe3\x83\x97\xe3\x83\xaa\xe3\x82\xbb\xe3\x83\x83\xe3\x83\x88\xe3\x82\x92\xe5\x89\x8a\xe9\x99\xa4\xe3\x81\x97\xe3\x81\xbe\xe3\x81\x99\xe3\x81\x8b\xef\xbc\x9f");  // このプリセットを削除しますか？
    stringsJa[StringKey::DialogYes] = juce::CharPointer_UTF8("\xe3\x81\xaf\xe3\x81\x84");  // はい
    stringsJa[StringKey::DialogNo] = juce::CharPointer_UTF8("\xe3\x81\x84\xe3\x81\x84\xe3\x81\x88");  // いいえ

    // Import
    stringsJa[StringKey::ImportFiles] = juce::CharPointer_UTF8("\xe3\x83\x95\xe3\x82\xa1\xe3\x82\xa4\xe3\x83\xab\xe3\x82\x92\xe3\x82\xa4\xe3\x83\xb3\xe3\x83\x9d\xe3\x83\xbc\xe3\x83\x88");  // ファイルをインポート
    stringsJa[StringKey::ImportSuccess] = juce::CharPointer_UTF8("\xe3\x82\xa4\xe3\x83\xb3\xe3\x83\x9d\xe3\x83\xbc\xe3\x83\x88\xe6\x88\x90\xe5\x8a\x9f");  // インポート成功
    stringsJa[StringKey::ImportFailed] = juce::CharPointer_UTF8("\xe3\x82\xa4\xe3\x83\xb3\xe3\x83\x9d\xe3\x83\xbc\xe3\x83\x88\xe5\xa4\xb1\xe6\x95\x97");  // インポート失敗
    stringsJa[StringKey::ImportFilesLoaded] = juce::CharPointer_UTF8("\xe3\x83\x95\xe3\x82\xa1\xe3\x82\xa4\xe3\x83\xab\xe8\xaa\xad\xe8\xbe\xbc\xe6\xb8\x88");  // ファイル読込済

    // Transport Bar
    stringsJa[StringKey::TransportPlay] = juce::CharPointer_UTF8("\xe5\x86\x8d\xe7\x94\x9f");  // 再生
    stringsJa[StringKey::TransportPause] = juce::CharPointer_UTF8("\xe4\xb8\x80\xe6\x99\x82\xe5\x81\x9c\xe6\xad\xa2");  // 一時停止
    stringsJa[StringKey::TransportStop] = juce::CharPointer_UTF8("\xe5\x81\x9c\xe6\xad\xa2");  // 停止

    // Presets
    stringsJa[StringKey::PresetDefault] = juce::CharPointer_UTF8("\xe3\x83\x87\xe3\x83\x95\xe3\x82\xa9\xe3\x83\xab\xe3\x83\x88");  // デフォルト
    stringsJa[StringKey::PresetSave] = juce::CharPointer_UTF8("\xe4\xbf\x9d\xe5\xad\x98");  // 保存
    stringsJa[StringKey::PresetDelete] = juce::CharPointer_UTF8("\xe5\x89\x8a\xe9\x99\xa4");  // 削除
    stringsJa[StringKey::PresetSelect] = juce::CharPointer_UTF8("\xe3\x83\x97\xe3\x83\xaa\xe3\x82\xbb\xe3\x83\x83\xe3\x83\x88\xe9\x81\xb8\xe6\x8a\x9e");  // プリセット選択

    // Star Rating
    stringsJa[StringKey::RatingLabel] = juce::CharPointer_UTF8("\xe8\xa9\x95\xe4\xbe\xa1");  // 評価

    // Guess Dropdown
    stringsJa[StringKey::GuessSelect] = juce::CharPointer_UTF8("\xe3\x83\x88\xe3\x83\xa9\xe3\x83\x83\xe3\x82\xaf\xe3\x82\x92\xe9\x81\xb8\xe6\x8a\x9e...");  // トラックを選択...
    stringsJa[StringKey::GuessNone] = juce::CharPointer_UTF8("\xe3\x81\xaa\xe3\x81\x97");  // なし

    //==========================================================================
    // SIMPLIFIED CHINESE (简体中文)
    //==========================================================================

    // Header Bar
    stringsZhCn[StringKey::NowPlaying] = juce::CharPointer_UTF8("\xe6\xad\xa3\xe5\x9c\xa8\xe6\x92\xad\xe6\x94\xbe\xef\xbc\x9a");  // 正在播放：
    stringsZhCn[StringKey::NoFileLoaded] = juce::CharPointer_UTF8("\xe6\x9c\xaa\xe5\x8a\xa0\xe8\xbd\xbd\xe6\x96\x87\xe4\xbb\xb6");  // 未加载文件

    // Mode Selector
    stringsZhCn[StringKey::ModeStars] = juce::CharPointer_UTF8("\xe8\xaf\x84\xe5\x88\x86\xe6\xa8\xa1\xe5\xbc\x8f");  // 评分模式
    stringsZhCn[StringKey::ModeGuess] = juce::CharPointer_UTF8("\xe9\x85\x8d\xe5\xaf\xb9");  // 配对
    stringsZhCn[StringKey::ModeQA] = juce::CharPointer_UTF8("\xe4\xbd\xa0\xe9\x97\xae\xe6\x88\x91\xe7\xad\x94");  // 你问我答

    // Control Panel - Buttons
    stringsZhCn[StringKey::ButtonShuffle] = juce::CharPointer_UTF8("\xe6\xb4\x97\xe7\x89\x8c");  // 洗牌
    stringsZhCn[StringKey::ButtonReveal] = juce::CharPointer_UTF8("\xe6\x8f\xad\xe7\xa4\xba");  // 揭示
    stringsZhCn[StringKey::ButtonReset] = juce::CharPointer_UTF8("\xe9\x87\x8d\xe7\xbd\xae");  // 重置
    stringsZhCn[StringKey::ButtonNextRound] = juce::CharPointer_UTF8("\xe4\xb8\x8b\xe4\xb8\x80\xe5\x9b\x9e\xe5\x90\x88");  // 下一回合
    stringsZhCn[StringKey::ButtonSubmit] = juce::CharPointer_UTF8("\xe6\x8f\x90\xe4\xba\xa4");  // 提交

    // Control Panel - Labels
    stringsZhCn[StringKey::LabelRounds] = juce::CharPointer_UTF8("\xe5\x9b\x9e\xe5\x90\x88\xe6\x95\xb0");  // 回合数
    stringsZhCn[StringKey::LabelAutoGain] = juce::CharPointer_UTF8("\xe8\x87\xaa\xe5\x8a\xa8\xe5\xa2\x9e\xe7\x9b\x8a");  // 自动增益
    stringsZhCn[StringKey::LabelQuestions] = juce::CharPointer_UTF8("\xe9\x97\xae\xe9\xa2\x98\xe6\x95\xb0");  // 问题数
    stringsZhCn[StringKey::LabelCards] = juce::CharPointer_UTF8("\xe5\x8d\xa1\xe7\x89\x87\xe6\x95\xb0");  // 卡片数

    // Control Panel - Status
    stringsZhCn[StringKey::StatusReady] = juce::CharPointer_UTF8("\xe5\xb0\xb1\xe7\xbb\xaa");  // 就绪
    stringsZhCn[StringKey::StatusShuffled] = juce::CharPointer_UTF8("\xe5\xb7\xb2\xe6\xb4\x97\xe7\x89\x8c");  // 已洗牌
    stringsZhCn[StringKey::StatusRevealed] = juce::CharPointer_UTF8("\xe5\xb7\xb2\xe6\x8f\xad\xe7\xa4\xba");  // 已揭示
    stringsZhCn[StringKey::StatusRound] = juce::CharPointer_UTF8("\xe7\xac\xac");  // 第
    stringsZhCn[StringKey::StatusOf] = juce::CharPointer_UTF8("\xe5\x9b\x9e\xe5\x90\x88\xef\xbc\x8c\xe5\x85\xb1");  // 回合，共
    stringsZhCn[StringKey::StatusDetecting] = juce::CharPointer_UTF8("\xe6\xa3\x80\xe6\xb5\x8b\xe4\xb8\xad...");  // 检测中...
    stringsZhCn[StringKey::StatusWaiting] = juce::CharPointer_UTF8("\xe7\xad\x89\xe5\xbe\x85\xe4\xb8\xad...");  // 等待中...

    // Results Panel
    stringsZhCn[StringKey::ResultsTitle] = juce::CharPointer_UTF8("\xe7\xbb\x93\xe6\x9e\x9c");  // 结果
    stringsZhCn[StringKey::ResultsEmpty] = juce::CharPointer_UTF8("\xe5\xb0\x9a\xe6\x97\xa0\xe7\xbb\x93\xe6\x9e\x9c");  // 尚无结果
    stringsZhCn[StringKey::ResultsTrack] = juce::CharPointer_UTF8("\xe9\x9f\xb3\xe8\xbd\xa8");  // 音轨
    stringsZhCn[StringKey::ResultsScore] = juce::CharPointer_UTF8("\xe5\x88\x86\xe6\x95\xb0");  // 分数
    stringsZhCn[StringKey::ResultsAverage] = juce::CharPointer_UTF8("\xe5\xb9\xb3\xe5\x9d\x87");  // 平均
    stringsZhCn[StringKey::ResultsCorrect] = juce::CharPointer_UTF8("\xe6\xad\xa3\xe7\xa1\xae");  // 正确
    stringsZhCn[StringKey::ResultsIncorrect] = juce::CharPointer_UTF8("\xe9\x94\x99\xe8\xaf\xaf");  // 错误
    stringsZhCn[StringKey::ResultsTotal] = juce::CharPointer_UTF8("\xe6\x80\xbb\xe8\xae\xa1");  // 总计

    // Question Banner (Q&A mode)
    stringsZhCn[StringKey::QAQuestion] = juce::CharPointer_UTF8("\xe9\x97\xae\xe9\xa2\x98");  // 问题
    stringsZhCn[StringKey::QASelectCard] = juce::CharPointer_UTF8("\xe8\xaf\xb7\xe9\x80\x89\xe6\x8b\xa9\xe4\xb8\x80\xe5\xbc\xa0\xe5\x8d\xa1\xe7\x89\x87");  // 请选择一张卡片
    stringsZhCn[StringKey::QAWhichBetter] = juce::CharPointer_UTF8("\xe5\x93\xaa\xe4\xb8\xaa\xe5\x90\xac\xe8\xb5\xb7\xe6\x9d\xa5\xe6\x9b\xb4\xe5\xa5\xbd\xef\xbc\x9f");  // 哪个听起来更好？
    stringsZhCn[StringKey::QAPreference] = juce::CharPointer_UTF8("\xe4\xbd\xa0\xe7\x9a\x84\xe5\x81\x8f\xe5\xa5\xbd");  // 你的偏好

    // Poker Card
    stringsZhCn[StringKey::CardDropHere] = juce::CharPointer_UTF8("\xe6\x8b\x96\xe6\x8b\xbd\xe9\x9f\xb3\xe9\xa2\x91\xe6\x96\x87\xe4\xbb\xb6\xe8\x87\xb3\xe6\xad\xa4");  // 拖拽音频文件至此
    stringsZhCn[StringKey::CardDragFile] = juce::CharPointer_UTF8("\xe6\x8b\x96\xe6\x8b\xbd\xe9\x9f\xb3\xe9\xa2\x91");  // 拖拽音频
    stringsZhCn[StringKey::CardTrack] = juce::CharPointer_UTF8("\xe9\x9f\xb3\xe8\xbd\xa8");  // 音轨

    // Card Count Control
    stringsZhCn[StringKey::CardCountLabel] = juce::CharPointer_UTF8("\xe5\x8d\xa1\xe7\x89\x87\xe6\x95\xb0\xe9\x87\x8f");  // 卡片数量

    // Settings Panel
    stringsZhCn[StringKey::SettingsTitle] = juce::CharPointer_UTF8("\xe8\xae\xbe\xe7\xbd\xae");  // 设置
    stringsZhCn[StringKey::SettingsLanguage] = juce::CharPointer_UTF8("\xe8\xaf\xad\xe8\xa8\x80");  // 语言
    stringsZhCn[StringKey::SettingsTheme] = juce::CharPointer_UTF8("\xe4\xb8\xbb\xe9\xa2\x98");  // 主题
    stringsZhCn[StringKey::SettingsThemeLight] = juce::CharPointer_UTF8("\xe6\xb5\x85\xe8\x89\xb2");  // 浅色
    stringsZhCn[StringKey::SettingsThemeDark] = juce::CharPointer_UTF8("\xe6\xb7\xb1\xe8\x89\xb2");  // 深色
    stringsZhCn[StringKey::SettingsClose] = juce::CharPointer_UTF8("\xe5\x85\xb3\xe9\x97\xad");  // 关闭

    // About Dialog
    stringsZhCn[StringKey::AboutTitle] = juce::CharPointer_UTF8("\xe5\x85\xb3\xe4\xba\x8e");  // 关于
    stringsZhCn[StringKey::AboutFreeSoftware] = juce::CharPointer_UTF8("\xe5\x85\x8d\xe8\xb4\xb9\xe8\xbd\xaf\xe4\xbb\xb6");  // 免费软件
    stringsZhCn[StringKey::AboutNoLicense] = juce::CharPointer_UTF8("\xe6\x97\xa0\xe9\x9c\x80\xe6\x8e\x88\xe6\x9d\x83\xe5\xaf\x86\xe9\x92\xa5");  // 无需授权密钥
    stringsZhCn[StringKey::AboutVersion] = juce::CharPointer_UTF8("\xe7\x89\x88\xe6\x9c\xac\xef\xbc\x9a");  // 版本：
    stringsZhCn[StringKey::AboutDeveloper] = juce::CharPointer_UTF8("\xe5\xbc\x80\xe5\x8f\x91\xe8\x80\x85\xef\xbc\x9a");  // 开发者：
    stringsZhCn[StringKey::AboutVisitWebsite] = juce::CharPointer_UTF8("\xe8\xae\xbf\xe9\x97\xae\xe5\xae\x98\xe6\x96\xb9\xe7\xbd\x91\xe7\xab\x99");  // 访问官方网站

    // Quick Start Guide
    stringsZhCn[StringKey::GuideTitle] = juce::CharPointer_UTF8("\xe5\xbf\xab\xe9\x80\x9f\xe5\x85\xa5\xe9\x97\xa8\xe6\x8c\x87\xe5\x8d\x97");  // 快速入门指南
    stringsZhCn[StringKey::GuideGotIt] = juce::CharPointer_UTF8("\xe6\x88\x91\xe7\x9f\xa5\xe9\x81\x93\xe4\xba\x86\xef\xbc\x81");  // 我知道了！
    stringsZhCn[StringKey::GuideDescription] = juce::CharPointer_UTF8("Blind Card \xe8\xae\xa9\xe4\xbd\xa0\xe5\x9c\xa8\xe4\xb8\x8d\xe7\x9f\xa5\xe9\x81\x93\xe9\x9f\xb3\xe8\xbd\xa8\xe8\xba\xab\xe4\xbb\xbd\xe7\x9a\x84\xe6\x83\x85\xe5\x86\xb5\xe4\xb8\x8b\xe6\xaf\x94\xe8\xbe\x83\xe9\x9f\xb3\xe9\xa2\x91\xef\xbc\x8c\xe6\xb6\x88\xe9\x99\xa4\xe5\x86\xb3\xe7\xad\x96\xe4\xb8\xad\xe7\x9a\x84\xe5\x81\x8f\xe8\xa7\x81\xe3\x80\x82");  // Blind Card 让你在不知道音轨身份的情况下比较音频，消除决策中的偏见。

    // Steps (full detailed text)
    stringsZhCn[StringKey::GuideStep1Full] = juce::CharPointer_UTF8("\xe5\x9c\xa8\xe6\xaf\x8f\xe4\xb8\xaa\xe8\xa6\x81\xe6\xaf\x94\xe8\xbe\x83\xe7\x9a\x84\xe9\x9f\xb3\xe8\xbd\xa8\xe4\xb8\x8a\xe6\x8f\x92\xe5\x85\xa5 Blind Card (2-8 \xe4\xb8\xaa\xe9\x9f\xb3\xe8\xbd\xa8)");  // 在每个要比较的音轨上插入 Blind Card (2-8 个音轨)
    stringsZhCn[StringKey::GuideStep2Intro] = juce::CharPointer_UTF8("\xe9\x80\x89\xe6\x8b\xa9\xe8\xaf\x84\xe5\x88\x86\xe6\xa8\xa1\xe5\xbc\x8f\xef\xbc\x9a");  // 选择评分模式：
    stringsZhCn[StringKey::GuideStep3Full] = juce::CharPointer_UTF8("\xe7\x82\xb9\xe5\x87\xbb\xe3\x80\x8c\xe6\xb4\x97\xe7\x89\x8c\xe3\x80\x8d\xe9\x9a\x8f\xe6\x9c\xba\xe6\x8e\x92\xe5\x88\x97\xe5\xb9\xb6\xe9\x9a\x90\xe8\x97\x8f\xe9\x9f\xb3\xe8\xbd\xa8\xe8\xba\xab\xe4\xbb\xbd");  // 点击「洗牌」随机排列并隐藏音轨身份
    stringsZhCn[StringKey::GuideStep4Full] = juce::CharPointer_UTF8("\xe7\x82\xb9\xe5\x87\xbb\xe5\x8d\xa1\xe7\x89\x87\xe8\x81\x86\xe5\x90\xac\xe5\xb9\xb6\xe8\xaf\x84\xe5\x88\x86\xef\xbc\x88\xe6\x88\x96\xe4\xbd\xbf\xe7\x94\xa8\xe6\x96\xb9\xe5\x90\x91\xe9\x94\xae\xef\xbc\x89");  // 点击卡片聆听并评分（或使用方向键）
    stringsZhCn[StringKey::GuideStep5Full] = juce::CharPointer_UTF8("\xe7\x82\xb9\xe5\x87\xbb\xe3\x80\x8c\xe6\x8f\xad\xe7\xa4\xba\xe3\x80\x8d\xe6\x9f\xa5\xe7\x9c\x8b\xe7\xbb\x93\xe6\x9e\x9c\xe5\x92\x8c\xe5\xae\x9e\xe9\x99\x85\xe9\x9f\xb3\xe8\xbd\xa8\xe5\x90\x8d\xe7\xa7\xb0");  // 点击「揭示」查看结果和实际音轨名称
    stringsZhCn[StringKey::GuideStep6Full] = juce::CharPointer_UTF8("\xe7\x82\xb9\xe5\x87\xbb\xe3\x80\x8c\xe9\x87\x8d\xe7\xbd\xae\xe3\x80\x8d\xe5\xbc\x80\xe5\xa7\x8b\xe6\x96\xb0\xe7\x9a\x84\xe6\xb5\x8b\xe8\xaf\x95");  // 点击「重置」开始新的测试

    // Mode descriptions in guide
    stringsZhCn[StringKey::GuideModeStarsDesc] = juce::CharPointer_UTF8("\xe4\xb8\xba\xe6\xaf\x8f\xe4\xb8\xaa\xe9\x9f\xb3\xe8\xbd\xa8\xe8\xaf\x84\xe5\x88\x86 1-5 \xe9\xa2\x97\xe6\x98\x9f");  // 为每个音轨评分 1-5 颗星
    stringsZhCn[StringKey::GuideModeGuessDesc] = juce::CharPointer_UTF8("\xe8\xaf\x86\xe5\x88\xab\xe6\xaf\x8f\xe5\xbc\xa0\xe5\x8d\xa1\xe7\x89\x87\xe4\xb8\x8a\xe6\x98\xaf\xe5\x93\xaa\xe4\xb8\xaa\xe6\x8f\x92\xe4\xbb\xb6/\xe6\xb7\xb7\xe9\x9f\xb3");  // 识别每张卡片上是哪个插件/混音
    stringsZhCn[StringKey::GuideModeQADesc] = juce::CharPointer_UTF8("\xe6\xa0\xb9\xe6\x8d\xae\xe6\x8f\x90\xe7\xa4\xba\xe6\x89\xbe\xe5\x87\xba\xe7\x89\xb9\xe5\xae\x9a\xe9\x9f\xb3\xe8\xbd\xa8");  // 根据提示找出特定音轨

    // Tip box
    stringsZhCn[StringKey::GuideTip] = juce::CharPointer_UTF8("\xe6\x8f\x90\xe7\xa4\xba");  // 提示
    stringsZhCn[StringKey::GuideTipText1] = juce::CharPointer_UTF8("\xe5\x90\xaf\xe7\x94\xa8\xe3\x80\x8c\xe8\x87\xaa\xe5\x8a\xa8\xe5\xa2\x9e\xe7\x9b\x8a\xe3\x80\x8d\xe6\x9d\xa5\xe5\x93\x8d\xe5\xba\xa6\xe5\x8c\xb9\xe9\x85\x8d\xe6\x89\x80\xe6\x9c\x89\xe9\x9f\xb3\xe8\xbd\xa8\xef\xbc\x8c");  // 启用「自动增益」来响度匹配所有音轨，
    stringsZhCn[StringKey::GuideTipText2] = juce::CharPointer_UTF8("\xe7\xa1\xae\xe4\xbf\x9d\xe5\x85\xac\xe5\xb9\xb3\xe6\xaf\x94\xe8\xbe\x83\xef\xbc\x8c\xe4\xb8\x8d\xe5\x8f\x97\xe9\x9f\xb3\xe9\x87\x8f\xe5\x81\x8f\xe8\xa7\x81\xe5\xbd\xb1\xe5\x93\x8d\xe3\x80\x82");  // 确保公平比较，不受音量偏见影响。

    // Keyboard shortcuts
    stringsZhCn[StringKey::GuideKeyboardShortcuts] = juce::CharPointer_UTF8("\xe9\x94\xae\xe7\x9b\x98\xe5\xbf\xab\xe6\x8d\xb7\xe9\x94\xae");  // 键盘快捷键
    stringsZhCn[StringKey::GuideShortcutSelectCard] = juce::CharPointer_UTF8("\xe7\x9b\xb4\xe6\x8e\xa5\xe9\x80\x89\xe6\x8b\xa9\xe5\x8d\xa1\xe7\x89\x87");  // 直接选择卡片
    stringsZhCn[StringKey::GuideShortcutNextPrev] = juce::CharPointer_UTF8("\xe4\xb8\x8b\xe4\xb8\x80\xe5\xbc\xa0 / \xe4\xb8\x8a\xe4\xb8\x80\xe5\xbc\xa0\xe5\x8d\xa1\xe7\x89\x87");  // 下一张 / 上一张卡片
    stringsZhCn[StringKey::GuideShortcutBrackets] = juce::CharPointer_UTF8("\xe4\xb8\x8a\xe4\xb8\x80\xe5\xbc\xa0 / \xe4\xb8\x8b\xe4\xb8\x80\xe5\xbc\xa0\xe5\x8d\xa1\xe7\x89\x87");  // 上一张 / 下一张卡片
    stringsZhCn[StringKey::GuideShortcutArrows] = juce::CharPointer_UTF8("\xe5\xaf\xbc\xe8\x88\xaa\xe5\x8d\xa1\xe7\x89\x87\xef\xbc\x88\xe5\x9c\xa8 Logic Pro \xe4\xb8\xad\xe5\x8f\xaf\xe8\x83\xbd\xe4\xb8\x8d\xe8\xb5\xb7\xe4\xbd\x9c\xe7\x94\xa8\xef\xbc\x89");  // 导航卡片（在 Logic Pro 中可能不起作用）

    // Dialogs
    stringsZhCn[StringKey::DialogSavePreset] = juce::CharPointer_UTF8("\xe4\xbf\x9d\xe5\xad\x98\xe9\xa2\x84\xe8\xae\xbe");  // 保存预设
    stringsZhCn[StringKey::DialogEnterName] = juce::CharPointer_UTF8("\xe8\xaf\xb7\xe8\xbe\x93\xe5\x85\xa5\xe9\xa2\x84\xe8\xae\xbe\xe5\x90\x8d\xe7\xa7\xb0\xef\xbc\x9a");  // 请输入预设名称：
    stringsZhCn[StringKey::DialogSave] = juce::CharPointer_UTF8("\xe4\xbf\x9d\xe5\xad\x98");  // 保存
    stringsZhCn[StringKey::DialogCancel] = juce::CharPointer_UTF8("\xe5\x8f\x96\xe6\xb6\x88");  // 取消
    stringsZhCn[StringKey::DialogDelete] = juce::CharPointer_UTF8("\xe5\x88\xa0\xe9\x99\xa4");  // 删除
    stringsZhCn[StringKey::DialogConfirmDelete] = juce::CharPointer_UTF8("\xe7\xa1\xae\xe5\xae\x9a\xe8\xa6\x81\xe5\x88\xa0\xe9\x99\xa4\xe6\xad\xa4\xe9\xa2\x84\xe8\xae\xbe\xe5\x90\x97\xef\xbc\x9f");  // 确定要删除此预设吗？
    stringsZhCn[StringKey::DialogYes] = juce::CharPointer_UTF8("\xe6\x98\xaf");  // 是
    stringsZhCn[StringKey::DialogNo] = juce::CharPointer_UTF8("\xe5\x90\xa6");  // 否

    // Import
    stringsZhCn[StringKey::ImportFiles] = juce::CharPointer_UTF8("\xe5\xaf\xbc\xe5\x85\xa5\xe6\x96\x87\xe4\xbb\xb6");  // 导入文件
    stringsZhCn[StringKey::ImportSuccess] = juce::CharPointer_UTF8("\xe5\xaf\xbc\xe5\x85\xa5\xe6\x88\x90\xe5\x8a\x9f");  // 导入成功
    stringsZhCn[StringKey::ImportFailed] = juce::CharPointer_UTF8("\xe5\xaf\xbc\xe5\x85\xa5\xe5\xa4\xb1\xe8\xb4\xa5");  // 导入失败
    stringsZhCn[StringKey::ImportFilesLoaded] = juce::CharPointer_UTF8("\xe4\xb8\xaa\xe6\x96\x87\xe4\xbb\xb6\xe5\xb7\xb2\xe5\x8a\xa0\xe8\xbd\xbd");  // 个文件已加载

    // Transport Bar
    stringsZhCn[StringKey::TransportPlay] = juce::CharPointer_UTF8("\xe6\x92\xad\xe6\x94\xbe");  // 播放
    stringsZhCn[StringKey::TransportPause] = juce::CharPointer_UTF8("\xe6\x9a\x82\xe5\x81\x9c");  // 暂停
    stringsZhCn[StringKey::TransportStop] = juce::CharPointer_UTF8("\xe5\x81\x9c\xe6\xad\xa2");  // 停止

    // Presets
    stringsZhCn[StringKey::PresetDefault] = juce::CharPointer_UTF8("\xe9\xa2\x84\xe8\xae\xbe");  // 预设
    stringsZhCn[StringKey::PresetSave] = juce::CharPointer_UTF8("\xe4\xbf\x9d\xe5\xad\x98");  // 保存
    stringsZhCn[StringKey::PresetDelete] = juce::CharPointer_UTF8("\xe5\x88\xa0\xe9\x99\xa4");  // 删除
    stringsZhCn[StringKey::PresetSelect] = juce::CharPointer_UTF8("\xe9\x80\x89\xe6\x8b\xa9\xe9\xa2\x84\xe8\xae\xbe");  // 选择预设

    // Star Rating
    stringsZhCn[StringKey::RatingLabel] = juce::CharPointer_UTF8("\xe8\xaf\x84\xe5\x88\x86");  // 评分

    // Guess Dropdown
    stringsZhCn[StringKey::GuessSelect] = juce::CharPointer_UTF8("\xe9\x80\x89\xe6\x8b\xa9\xe9\x9f\xb3\xe8\xbd\xa8...");  // 选择音轨...
    stringsZhCn[StringKey::GuessNone] = juce::CharPointer_UTF8("\xe6\x97\xa0");  // 无

    //==========================================================================
    // KOREAN (한국어)
    //==========================================================================

    // Header Bar
    stringsKo[StringKey::NowPlaying] = juce::CharPointer_UTF8("\xec\x9e\xac\xec\x83\x9d \xec\xa4\x91:");  // 재생 중:
    stringsKo[StringKey::NoFileLoaded] = juce::CharPointer_UTF8("\xed\x8c\x8c\xec\x9d\xbc \xec\x97\x86\xec\x9d\x8c");  // 파일 없음

    // Mode Selector
    stringsKo[StringKey::ModeStars] = juce::CharPointer_UTF8("\xeb\xb3\x84\xec\xa0\x90 \xeb\xaa\xa8\xeb\x93\x9c");  // 별점 모드
    stringsKo[StringKey::ModeGuess] = juce::CharPointer_UTF8("\xeb\xa7\x9e\xec\xb6\x94\xea\xb8\xb0");  // 맞추기
    stringsKo[StringKey::ModeQA] = juce::CharPointer_UTF8("\xec\xa7\x88\xeb\xac\xb8");  // 질문

    // Control Panel - Buttons
    stringsKo[StringKey::ButtonShuffle] = juce::CharPointer_UTF8("\xec\x84\x9e\xea\xb8\xb0");  // 섞기
    stringsKo[StringKey::ButtonReveal] = juce::CharPointer_UTF8("\xea\xb3\xb5\xea\xb0\x9c");  // 공개
    stringsKo[StringKey::ButtonReset] = juce::CharPointer_UTF8("\xec\xb4\x88\xea\xb8\xb0\xed\x99\x94");  // 초기화
    stringsKo[StringKey::ButtonNextRound] = juce::CharPointer_UTF8("\xeb\x8b\xa4\xec\x9d\x8c \xeb\x9d\xbc\xec\x9a\xb4\xeb\x93\x9c");  // 다음 라운드
    stringsKo[StringKey::ButtonSubmit] = juce::CharPointer_UTF8("\xec\xa0\x9c\xec\xb6\x9c");  // 제출

    // Control Panel - Labels
    stringsKo[StringKey::LabelRounds] = juce::CharPointer_UTF8("\xeb\x9d\xbc\xec\x9a\xb4\xeb\x93\x9c \xec\x88\x98");  // 라운드 수
    stringsKo[StringKey::LabelAutoGain] = juce::CharPointer_UTF8("\xec\x9e\x90\xeb\x8f\x99 \xea\xb2\x8c\xec\x9d\xb8");  // 자동 게인
    stringsKo[StringKey::LabelQuestions] = juce::CharPointer_UTF8("\xec\xa7\x88\xeb\xac\xb8 \xec\x88\x98");  // 질문 수
    stringsKo[StringKey::LabelCards] = juce::CharPointer_UTF8("\xec\xb9\xb4\xeb\x93\x9c \xec\x88\x98");  // 카드 수

    // Control Panel - Status
    stringsKo[StringKey::StatusReady] = juce::CharPointer_UTF8("\xec\xa4\x80\xeb\xb9\x84 \xec\x99\x84\xeb\xa3\x8c");  // 준비 완료
    stringsKo[StringKey::StatusShuffled] = juce::CharPointer_UTF8("\xec\x84\x9e\xec\x9d\x8c");  // 섞음
    stringsKo[StringKey::StatusRevealed] = juce::CharPointer_UTF8("\xea\xb3\xb5\xea\xb0\x9c\xeb\x90\xa8");  // 공개됨
    stringsKo[StringKey::StatusRound] = juce::CharPointer_UTF8("\xeb\x9d\xbc\xec\x9a\xb4\xeb\x93\x9c");  // 라운드
    stringsKo[StringKey::StatusOf] = "/";  // /
    stringsKo[StringKey::StatusDetecting] = juce::CharPointer_UTF8("\xea\xb0\x90\xec\xa7\x80 \xec\xa4\x91...");  // 감지 중...
    stringsKo[StringKey::StatusWaiting] = juce::CharPointer_UTF8("\xeb\x8c\x80\xea\xb8\xb0 \xec\xa4\x91...");  // 대기 중...

    // Results Panel
    stringsKo[StringKey::ResultsTitle] = juce::CharPointer_UTF8("\xea\xb2\xb0\xea\xb3\xbc");  // 결과
    stringsKo[StringKey::ResultsEmpty] = juce::CharPointer_UTF8("\xea\xb2\xb0\xea\xb3\xbc \xec\x97\x86\xec\x9d\x8c");  // 결과 없음
    stringsKo[StringKey::ResultsTrack] = juce::CharPointer_UTF8("\xed\x8a\xb8\xeb\x9e\x99");  // 트랙
    stringsKo[StringKey::ResultsScore] = juce::CharPointer_UTF8("\xec\xa0\x90\xec\x88\x98");  // 점수
    stringsKo[StringKey::ResultsAverage] = juce::CharPointer_UTF8("\xed\x8f\x89\xea\xb7\xa0");  // 평균
    stringsKo[StringKey::ResultsCorrect] = juce::CharPointer_UTF8("\xec\xa0\x95\xeb\x8b\xb5");  // 정답
    stringsKo[StringKey::ResultsIncorrect] = juce::CharPointer_UTF8("\xec\x98\xa4\xeb\x8b\xb5");  // 오답
    stringsKo[StringKey::ResultsTotal] = juce::CharPointer_UTF8("\xec\xb4\x9d\xea\xb3\x84");  // 총계

    // Question Banner (Q&A mode)
    stringsKo[StringKey::QAQuestion] = juce::CharPointer_UTF8("\xec\xa7\x88\xeb\xac\xb8");  // 질문
    stringsKo[StringKey::QASelectCard] = juce::CharPointer_UTF8("\xec\xb9\xb4\xeb\x93\x9c\xeb\xa5\xbc \xec\x84\xa0\xed\x83\x9d\xed\x95\x98\xec\x84\xb8\xec\x9a\x94");  // 카드를 선택하세요
    stringsKo[StringKey::QAWhichBetter] = juce::CharPointer_UTF8("\xec\x96\xb4\xeb\x8a\x90 \xec\xaa\xbd\xec\x9d\xb4 \xeb\x8d\x94 \xec\xa2\x8b\xec\x9d\x80\xea\xb0\x80\xec\x9a\x94?");  // 어느 쪽이 더 좋은가요?
    stringsKo[StringKey::QAPreference] = juce::CharPointer_UTF8("\xeb\x8b\xb9\xec\x8b\xa0\xec\x9d\x98 \xec\x84\xa0\xed\x98\xb8");  // 당신의 선호

    // Poker Card
    stringsKo[StringKey::CardDropHere] = juce::CharPointer_UTF8("\xec\x97\xac\xea\xb8\xb0\xec\x97\x90 \xed\x8c\x8c\xec\x9d\xbc\xec\x9d\x84 \xeb\x93\x9c\xeb\xa1\xad\xed\x95\x98\xec\x84\xb8\xec\x9a\x94");  // 여기에 파일을 드롭하세요
    stringsKo[StringKey::CardDragFile] = juce::CharPointer_UTF8("\xeb\x93\x9c\xeb\x9e\x98\xea\xb7\xb8 \xec\x95\xa4 \xeb\x93\x9c\xeb\xa1\xad");  // 드래그 앤 드롭
    stringsKo[StringKey::CardTrack] = juce::CharPointer_UTF8("\xed\x8a\xb8\xeb\x9e\x99");  // 트랙

    // Card Count Control
    stringsKo[StringKey::CardCountLabel] = juce::CharPointer_UTF8("\xec\xb9\xb4\xeb\x93\x9c \xec\x88\x98");  // 카드 수

    // Settings Panel
    stringsKo[StringKey::SettingsTitle] = juce::CharPointer_UTF8("\xec\x84\xa4\xec\xa0\x95");  // 설정
    stringsKo[StringKey::SettingsLanguage] = juce::CharPointer_UTF8("\xec\x96\xb8\xec\x96\xb4");  // 언어
    stringsKo[StringKey::SettingsTheme] = juce::CharPointer_UTF8("\xed\x85\x8c\xeb\xa7\x88");  // 테마
    stringsKo[StringKey::SettingsThemeLight] = juce::CharPointer_UTF8("\xeb\x9d\xbc\xec\x9d\xb4\xed\x8a\xb8");  // 라이트
    stringsKo[StringKey::SettingsThemeDark] = juce::CharPointer_UTF8("\xeb\x8b\xa4\xed\x81\xac");  // 다크
    stringsKo[StringKey::SettingsClose] = juce::CharPointer_UTF8("\xeb\x8b\xab\xea\xb8\xb0");  // 닫기

    // About Dialog
    stringsKo[StringKey::AboutTitle] = juce::CharPointer_UTF8("\xec\xa0\x95\xeb\xb3\xb4");  // 정보
    stringsKo[StringKey::AboutFreeSoftware] = juce::CharPointer_UTF8("\xeb\xac\xb4\xeb\xa3\x8c \xec\x86\x8c\xed\x94\x84\xed\x8a\xb8\xec\x9b\xa8\xec\x96\xb4");  // 무료 소프트웨어
    stringsKo[StringKey::AboutNoLicense] = juce::CharPointer_UTF8("\xeb\x9d\xbc\xec\x9d\xb4\xec\x84\xa0\xec\x8a\xa4 \xed\x82\xa4 \xed\x95\x84\xec\x9a\x94 \xec\x97\x86\xec\x9d\x8c");  // 라이선스 키 필요 없음
    stringsKo[StringKey::AboutVersion] = juce::CharPointer_UTF8("\xeb\xb2\x84\xec\xa0\x84:");  // 버전:
    stringsKo[StringKey::AboutDeveloper] = juce::CharPointer_UTF8("\xea\xb0\x9c\xeb\xb0\x9c\xec\x9e\x90:");  // 개발자:
    stringsKo[StringKey::AboutVisitWebsite] = juce::CharPointer_UTF8("\xea\xb3\xb5\xec\x8b\x9d \xec\x9b\xb9\xec\x82\xac\xec\x9d\xb4\xed\x8a\xb8 \xeb\xb0\xa9\xeb\xac\xb8");  // 공식 웹사이트 방문

    // Quick Start Guide
    stringsKo[StringKey::GuideTitle] = juce::CharPointer_UTF8("\xeb\xb9\xa0\xeb\xa5\xb8 \xec\x8b\x9c\xec\x9e\x91 \xea\xb0\x80\xec\x9d\xb4\xeb\x93\x9c");  // 빠른 시작 가이드
    stringsKo[StringKey::GuideGotIt] = juce::CharPointer_UTF8("\xec\x95\x8c\xea\xb2\xa0\xec\x8a\xb5\xeb\x8b\x88\xeb\x8b\xa4!");  // 알겠습니다!
    stringsKo[StringKey::GuideDescription] = juce::CharPointer_UTF8("Blind Card\xeb\x8a\x94 \xec\x96\xb4\xeb\x96\xa4 \xed\x8a\xb8\xeb\x9e\x99\xec\x9d\xb8\xec\xa7\x80 \xeb\xaa\xa8\xeb\xa5\xb4\xeb\x8a\x94 \xec\x83\x81\xed\x83\x9c\xec\x97\x90\xec\x84\x9c \xec\x98\xa4\xeb\x94\x94\xec\x98\xa4\xeb\xa5\xbc \xeb\xb9\x84\xea\xb5\x90\xed\x95\x98\xec\x97\xac \xed\x8e\xb8\xea\xb2\xac \xec\x97\x86\xeb\x8a\x94 \xea\xb2\xb0\xec\xa0\x95\xec\x9d\x84 \xeb\x82\xb4\xeb\xa6\xb4 \xec\x88\x98 \xec\x9e\x88\xea\xb2\x8c \xed\x95\xb4\xec\xa4\x8d\xeb\x8b\x88\xeb\x8b\xa4.");  // Blind Card는 어떤 트랙인지 모르는 상태에서 오디오를 비교하여 편견 없는 결정을 내릴 수 있게 해줍니다.

    // Steps (full detailed text)
    stringsKo[StringKey::GuideStep1Full] = juce::CharPointer_UTF8("\xeb\xb9\x84\xea\xb5\x90\xed\x95\x98\xeb\xa0\xa4\xeb\x8a\x94 \xea\xb0\x81 \xed\x8a\xb8\xeb\x9e\x99\xec\x97\x90 Blind Card\xeb\xa5\xbc \xec\x82\xbd\xec\x9e\x85\xed\x95\x98\xec\x84\xb8\xec\x9a\x94 (2-8 \xed\x8a\xb8\xeb\x9e\x99)");  // 비교하려는 각 트랙에 Blind Card를 삽입하세요 (2-8 트랙)
    stringsKo[StringKey::GuideStep2Intro] = juce::CharPointer_UTF8("\xed\x8f\x89\xea\xb0\x80 \xeb\xaa\xa8\xeb\x93\x9c\xeb\xa5\xbc \xec\x84\xa0\xed\x83\x9d\xed\x95\x98\xec\x84\xb8\xec\x9a\x94:");  // 평가 모드를 선택하세요:
    stringsKo[StringKey::GuideStep3Full] = juce::CharPointer_UTF8("\"\xec\x84\x9e\xea\xb8\xb0\"\xeb\xa5\xbc \xed\x81\xb4\xeb\xa6\xad\xed\x95\x98\xec\x97\xac \xeb\xac\xb4\xec\x9e\x91\xec\x9c\x84\xeb\xa1\x9c \xec\x84\x9e\xea\xb3\xa0 \xed\x8a\xb8\xeb\x9e\x99 \xec\x9d\xb4\xeb\xa6\x84\xec\x9d\x84 \xec\x88\xa8\xea\xb9\x81\xeb\x8b\x88\xeb\x8b\xa4");  // "섞기"를 클릭하여 무작위로 섞고 트랙 이름을 숨깁니다
    stringsKo[StringKey::GuideStep4Full] = juce::CharPointer_UTF8("\xec\xb9\xb4\xeb\x93\x9c\xeb\xa5\xbc \xed\x81\xb4\xeb\xa6\xad\xed\x95\x98\xec\x97\xac \xeb\x93\xa3\xea\xb3\xa0 \xed\x8f\x89\xea\xb0\x80\xed\x95\x98\xec\x84\xb8\xec\x9a\x94 (\xeb\x98\x90\xeb\x8a\x94 \xed\x99\x94\xec\x82\xb4\xed\x91\x9c \xed\x82\xa4 \xec\x82\xac\xec\x9a\xa9)");  // 카드를 클릭하여 듣고 평가하세요 (또는 화살표 키 사용)
    stringsKo[StringKey::GuideStep5Full] = juce::CharPointer_UTF8("\"\xea\xb3\xb5\xea\xb0\x9c\"\xeb\xa5\xbc \xed\x81\xb4\xeb\xa6\xad\xed\x95\x98\xec\x97\xac \xea\xb2\xb0\xea\xb3\xbc\xec\x99\x80 \xec\x8b\xa4\xec\xa0\x9c \xed\x8a\xb8\xeb\x9e\x99 \xec\x9d\xb4\xeb\xa6\x84\xec\x9d\x84 \xed\x99\x95\xec\x9d\xb8\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4");  // "공개"를 클릭하여 결과와 실제 트랙 이름을 확인합니다
    stringsKo[StringKey::GuideStep6Full] = juce::CharPointer_UTF8("\"\xec\xb4\x88\xea\xb8\xb0\xed\x99\x94\"\xeb\xa5\xbc \xed\x81\xb4\xeb\xa6\xad\xed\x95\x98\xec\x97\xac \xec\x83\x88 \xed\x85\x8c\xec\x8a\xa4\xed\x8a\xb8\xeb\xa5\xbc \xec\x8b\x9c\xec\x9e\x91\xed\x95\xa9\xeb\x8b\x88\xeb\x8b\xa4");  // "초기화"를 클릭하여 새 테스트를 시작합니다

    // Mode descriptions in guide
    stringsKo[StringKey::GuideModeStarsDesc] = juce::CharPointer_UTF8("\xea\xb0\x81 \xed\x8a\xb8\xeb\x9e\x99\xec\x97\x90 1-5\xea\xb0\x9c\xec\x9d\x98 \xeb\xb3\x84\xec\xa0\x90\xec\x9c\xbc\xeb\xa1\x9c \xed\x8f\x89\xea\xb0\x80");  // 각 트랙에 1-5개의 별점으로 평가
    stringsKo[StringKey::GuideModeGuessDesc] = juce::CharPointer_UTF8("\xea\xb0\x81 \xec\xb9\xb4\xeb\x93\x9c\xec\x9d\x98 \xed\x94\x8c\xeb\x9f\xac\xea\xb7\xb8\xec\x9d\xb8/\xeb\xaf\xb9\xec\x8a\xa4 \xec\x8b\x9d\xeb\xb3\x84");  // 각 카드의 플러그인/믹스 식별
    stringsKo[StringKey::GuideModeQADesc] = juce::CharPointer_UTF8("\xed\x94\x84\xeb\xa1\xac\xed\x94\x84\xed\x8a\xb8\xec\x97\x90 \xeb\x94\xb0\xeb\x9d\xbc \xed\x8a\xb9\xec\xa0\x95 \xed\x8a\xb8\xeb\x9e\x99 \xec\xb0\xbe\xea\xb8\xb0");  // 프롬프트에 따라 특정 트랙 찾기

    // Tip box
    stringsKo[StringKey::GuideTip] = juce::CharPointer_UTF8("\xed\x8c\x81");  // 팁
    stringsKo[StringKey::GuideTipText1] = juce::CharPointer_UTF8("\"\xec\x9e\x90\xeb\x8f\x99 \xea\xb2\x8c\xec\x9d\xb8\"\xec\x9d\x84 \xed\x99\x9c\xec\x84\xb1\xed\x99\x94\xed\x95\x98\xec\x97\xac \xeb\xaa\xa8\xeb\x93\xa0 \xed\x8a\xb8\xeb\x9e\x99\xec\x9d\x98 \xec\x9d\x8c\xeb\x9f\x89\xec\x9d\x84 \xeb\xa7\x9e\xec\xb6\x94\xea\xb3\xa0,");  // "자동 게인"을 활성화하여 모든 트랙의 음량을 맞추고,
    stringsKo[StringKey::GuideTipText2] = juce::CharPointer_UTF8("\xec\x9d\x8c\xeb\x9f\x89 \xed\x8e\xb8\xea\xb2\xac \xec\x97\x86\xec\x9d\xb4 \xea\xb3\xb5\xec\xa0\x95\xed\x95\x98\xea\xb2\x8c \xeb\xb9\x84\xea\xb5\x90\xed\x95\xa0 \xec\x88\x98 \xec\x9e\x88\xec\x8a\xb5\xeb\x8b\x88\xeb\x8b\xa4.");  // 음량 편견 없이 공정하게 비교할 수 있습니다.

    // Keyboard shortcuts
    stringsKo[StringKey::GuideKeyboardShortcuts] = juce::CharPointer_UTF8("\xed\x82\xa4\xeb\xb3\xb4\xeb\x93\x9c \xeb\x8b\xa8\xec\xb6\x95\xed\x82\xa4");  // 키보드 단축키
    stringsKo[StringKey::GuideShortcutSelectCard] = juce::CharPointer_UTF8("\xec\xb9\xb4\xeb\x93\x9c \xec\xa7\x81\xec\xa0\x91 \xec\x84\xa0\xed\x83\x9d");  // 카드 직접 선택
    stringsKo[StringKey::GuideShortcutNextPrev] = juce::CharPointer_UTF8("\xeb\x8b\xa4\xec\x9d\x8c / \xec\x9d\xb4\xec\xa0\x84 \xec\xb9\xb4\xeb\x93\x9c");  // 다음 / 이전 카드
    stringsKo[StringKey::GuideShortcutBrackets] = juce::CharPointer_UTF8("\xec\x9d\xb4\xec\xa0\x84 / \xeb\x8b\xa4\xec\x9d\x8c \xec\xb9\xb4\xeb\x93\x9c");  // 이전 / 다음 카드
    stringsKo[StringKey::GuideShortcutArrows] = juce::CharPointer_UTF8("\xec\xb9\xb4\xeb\x93\x9c \xed\x83\x90\xec\x83\x89 (Logic Pro\xec\x97\x90\xec\x84\x9c\xeb\x8a\x94 \xec\x9e\x91\xeb\x8f\x99\xed\x95\x98\xec\xa7\x80 \xec\x95\x8a\xec\x9d\x84 \xec\x88\x98 \xec\x9e\x88\xec\x9d\x8c)");  // 카드 탐색 (Logic Pro에서는 작동하지 않을 수 있음)

    // Dialogs
    stringsKo[StringKey::DialogSavePreset] = juce::CharPointer_UTF8("\xed\x94\x84\xeb\xa6\xac\xec\x85\x8b \xec\xa0\x80\xec\x9e\xa5");  // 프리셋 저장
    stringsKo[StringKey::DialogEnterName] = juce::CharPointer_UTF8("\xed\x94\x84\xeb\xa6\xac\xec\x85\x8b \xec\x9d\xb4\xeb\xa6\x84\xec\x9d\x84 \xec\x9e\x85\xeb\xa0\xa5\xed\x95\x98\xec\x84\xb8\xec\x9a\x94:");  // 프리셋 이름을 입력하세요:
    stringsKo[StringKey::DialogSave] = juce::CharPointer_UTF8("\xec\xa0\x80\xec\x9e\xa5");  // 저장
    stringsKo[StringKey::DialogCancel] = juce::CharPointer_UTF8("\xec\xb7\xa8\xec\x86\x8c");  // 취소
    stringsKo[StringKey::DialogDelete] = juce::CharPointer_UTF8("\xec\x82\xad\xec\xa0\x9c");  // 삭제
    stringsKo[StringKey::DialogConfirmDelete] = juce::CharPointer_UTF8("\xec\x9d\xb4 \xed\x94\x84\xeb\xa6\xac\xec\x85\x8b\xec\x9d\x84 \xec\x82\xad\xec\xa0\x9c\xed\x95\x98\xec\x8b\x9c\xea\xb2\xa0\xec\x8a\xb5\xeb\x8b\x88\xea\xb9\x8c?");  // 이 프리셋을 삭제하시겠습니까?
    stringsKo[StringKey::DialogYes] = juce::CharPointer_UTF8("\xec\x98\x88");  // 예
    stringsKo[StringKey::DialogNo] = juce::CharPointer_UTF8("\xec\x95\x84\xeb\x8b\x88\xec\x98\xa4");  // 아니오

    // Import
    stringsKo[StringKey::ImportFiles] = juce::CharPointer_UTF8("\xed\x8c\x8c\xec\x9d\xbc \xea\xb0\x80\xec\xa0\xb8\xec\x98\xa4\xea\xb8\xb0");  // 파일 가져오기
    stringsKo[StringKey::ImportSuccess] = juce::CharPointer_UTF8("\xea\xb0\x80\xec\xa0\xb8\xec\x98\xa4\xea\xb8\xb0 \xec\x84\xb1\xea\xb3\xb5");  // 가져오기 성공
    stringsKo[StringKey::ImportFailed] = juce::CharPointer_UTF8("\xea\xb0\x80\xec\xa0\xb8\xec\x98\xa4\xea\xb8\xb0 \xec\x8b\xa4\xed\x8c\xa8");  // 가져오기 실패
    stringsKo[StringKey::ImportFilesLoaded] = juce::CharPointer_UTF8("\xed\x8c\x8c\xec\x9d\xbc \xeb\xa1\x9c\xeb\x93\x9c\xeb\x90\xa8");  // 파일 로드됨

    // Transport Bar
    stringsKo[StringKey::TransportPlay] = juce::CharPointer_UTF8("\xec\x9e\xac\xec\x83\x9d");  // 재생
    stringsKo[StringKey::TransportPause] = juce::CharPointer_UTF8("\xec\x9d\xbc\xec\x8b\x9c\xec\xa0\x95\xec\xa7\x80");  // 일시정지
    stringsKo[StringKey::TransportStop] = juce::CharPointer_UTF8("\xec\xa0\x95\xec\xa7\x80");  // 정지

    // Presets
    stringsKo[StringKey::PresetDefault] = juce::CharPointer_UTF8("\xea\xb8\xb0\xeb\xb3\xb8");  // 기본
    stringsKo[StringKey::PresetSave] = juce::CharPointer_UTF8("\xec\xa0\x80\xec\x9e\xa5");  // 저장
    stringsKo[StringKey::PresetDelete] = juce::CharPointer_UTF8("\xec\x82\xad\xec\xa0\x9c");  // 삭제
    stringsKo[StringKey::PresetSelect] = juce::CharPointer_UTF8("\xed\x94\x84\xeb\xa6\xac\xec\x85\x8b \xec\x84\xa0\xed\x83\x9d");  // 프리셋 선택

    // Star Rating
    stringsKo[StringKey::RatingLabel] = juce::CharPointer_UTF8("\xed\x8f\x89\xec\xa0\x90");  // 평점

    // Guess Dropdown
    stringsKo[StringKey::GuessSelect] = juce::CharPointer_UTF8("\xed\x8a\xb8\xeb\x9e\x99 \xec\x84\xa0\xed\x83\x9d...");  // 트랙 선택...
    stringsKo[StringKey::GuessNone] = juce::CharPointer_UTF8("\xec\x97\x86\xec\x9d\x8c");  // 없음
}

} // namespace BlindCard
