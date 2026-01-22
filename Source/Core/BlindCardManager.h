#pragma once

#include "Types.h"
#include <juce_events/juce_events.h>
#include <juce_audio_processors/juce_audio_processors.h>

class BlindCardProcessor; // forward declaration (global namespace)

namespace blindcard
{

class BlindCardManager final : public juce::ChangeBroadcaster
{
public:
    BlindCardManager();
    ~BlindCardManager() override;

    // 實例註冊
    int registerInstance (BlindCardProcessor* instance, const juce::String& trackName);
    void unregisterInstance (BlindCardProcessor* instance);

    // 遊戲控制
    void setTotalRounds (int rounds);
    void shuffle();
    void nextRound();
    void reveal();
    void reset();

    // 卡牌操作
    void selectCard (int cardId);
    void deselectCard();
    void rateCard (int cardId, int stars);
    void setNote (int cardId, const juce::String& note);
    void setTrackName (int cardId, const juce::String& name);
    void setGuess (int cardId, int guessedTrackId);

    // 狀態查詢
    GamePhase getPhase() const;
    int getCurrentRound() const;
    int getTotalRounds() const;
    int getSelectedCardId() const;
    bool isCardSelected (int cardId) const;
    juce::Array<CardSlot> getCards() const;
    int getRegisteredCount() const;
    bool canJoinGame() const;

    // 當前播放軌道資訊（給 UI 顯示用）
    float getCurrentPlayingRMSdB() const;         // 當前選中卡片的即時 RMS
    juce::String getCurrentPlayingTrackName() const;  // 當前選中卡片的軌道名

    // Bypass 控制
    void setBypassAll (bool bypass);
    bool isBypassAll() const;

    // 評分模式
    void setRatingMode (RatingMode mode);
    RatingMode getRatingMode() const;

    // Q&A 模式
    void submitQAAnswer (int selectedCardId);
    void nextQAQuestion();
    void tickQACountdown();           // 倒數計時 tick（由 UI 每秒呼叫）
    void skipQACountdown();           // 跳過倒數直接下一題
    const QAState& getQAState() const;
    juce::String getCurrentQuestionTrackName() const;
    bool canStartQAMode() const;
    int getQAMaxQuestions() const;
    int getQACorrectAnswerCardId() const;  // 取得當前正確答案的卡牌 ID
    void setQAQuestionCount (int count);   // 設定 Q&A 問題數（用戶選擇）
    int getQAQuestionCount() const;        // 取得 Q&A 問題數設定

    // Level Matching - 校準流程
    void startCalibration();              // 開始校準（重置測量數據）
    void lockCalibration();               // 鎖定校準結果
    void setLevelMatchEnabled (bool enabled);
    bool isLevelMatchEnabled() const;
    bool isCalibrating() const;           // 是否正在校準中
    bool isCalibrated() const;            // 是否已完成校準
    std::pair<int, int> getCalibrationProgress() const;  // 回傳 (已測量數, 總數)
    float getCalibrationTimeRemaining() const;           // 回傳剩餘秒數

    // Level Matching - 內部使用
    void setMeasuredLUFS (int cardId, float lufs);
    void setManualGain (int cardId, float gainDb);
    void recalculateAutoGains();          // 使用中位數基準計算
    float getGainForCard (int cardId) const;
    void resetLevelMatching();

    // 測試用：加入假卡牌（Standalone 測試動畫用）
    void addTestCards (int count = 4);

    // 測試用：設定卡牌數量（會清除並重建）
    void setTestCardCount (int count);

    // Standalone 模式：不自動分配軌道名稱
    void setStandaloneMode (bool enabled);
    bool isStandaloneMode() const { return standaloneMode; }

private:
    GameState state;
    juce::Array<BlindCardProcessor*> instances;
    mutable juce::CriticalSection lock;
    bool bypassAll = false;
    bool standaloneMode = false;  // Standalone 模式不自動分配軌道名稱
    RatingMode ratingMode = RatingMode::Stars;  // 預設為星等模式

    // Level Matching 狀態
    bool levelMatchEnabled_ = false;      // 是否啟用 Level-Match（預設關閉）
    bool calibrating_ = false;            // 是否正在校準中
    bool calibrated_ = false;             // 是否已完成校準並鎖定
    juce::int64 calibrationStartTime_ = 0; // 校準開始時間 (ms)
    static constexpr float kCalibrationDuration = 10.0f; // 校準時間 (秒)

    void notifyListeners();
    void selectNextQAQuestion();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BlindCardManager)
};

// SharedResourcePointer 會自動管理生命週期
using SharedBlindCardManager = juce::SharedResourcePointer<BlindCardManager>;

} // namespace blindcard
