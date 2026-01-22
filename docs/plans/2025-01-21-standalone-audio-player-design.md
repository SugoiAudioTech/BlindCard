# Standalone Audio Player 設計文件

**日期**: 2025-01-21
**狀態**: 已核准，待實作

## 概述

為 BlindCard Standalone 版本新增音檔拖放與播放功能，讓用戶無需 DAW 即可進行盲測比較。

## 核心功能

1. **拖曳音檔到卡牌**：每張卡牌可載入獨立音檔
2. **同步播放**：所有音檔 sample-accurate 同步播放
3. **無縫切換**：點擊卡牌立即切換音源，播放位置不變
4. **Transport 控制**：播放/暫停、時間顯示、進度條拖曳
5. **卡牌數量管理**：用戶可增減卡牌數（2-8張）

## 設計決策

| 項目 | 決策 |
|------|------|
| 音檔分配 | 每張卡牌獨立音檔 |
| 同步精確度 | Sample-accurate |
| 點擊行為 | 切換模式（無縫切換，永遠只聽一個） |
| UI 位置 | Transport 在 HeaderBar 右上角 |
| 拖放提示 | 「Drop Here」覆蓋層 |
| 支援格式 | WAV, MP3, AIFF |
| 長度不同 | 以最長為準，較短的播完後靜音 |
| 預設卡牌數 | Standalone 預設 2 張 |

## 架構設計

### 整體佈局

```
┌─────────────────────────────────────────────────────────────────┐
│ HeaderBar                                                        │
│ [Logo] BlindCard    [Track]    [◀◀][▶][▶▶] 01:23/03:45 [━●━] [⚙]│
└─────────────────────────────────────────────────────────────────┘
│                      ModeSelector                                │
├─────────────────────────────────────────┬───────────────────────┤
│              PokerTable                 │    ControlPanel       │
│         (拖曳音檔到卡牌)                │  Cards: [－] 4 [＋]   │
│    ┌─────┐  ┌─────┐  ┌─────┐  ┌─────┐  │  Rounds: [－] 3 [＋]  │
│    │ ♠ A │  │ ♣ K │  │Drop │  │Drop │  │  [Shuffle] [Reveal]   │
│    │wav1 │  │wav2 │  │Here │  │Here │  │───────────────────────│
│    └─────┘  └─────┘  └─────┘  └─────┘  │    ResultsPanel       │
└─────────────────────────────────────────┴───────────────────────┘
```

### StandaloneAudioEngine

```cpp
class StandaloneAudioEngine {
    struct AudioSlot {
        int cardId;
        std::unique_ptr<juce::AudioFormatReaderSource> source;
        juce::File file;
        double lengthInSeconds;
        bool isLoaded;
    };

    std::vector<AudioSlot> slots;
    int activeSlotIndex = 0;
    juce::int64 playheadPosition = 0;
    double sampleRate = 44100.0;
    bool isPlaying = false;

    // 核心 API
    bool loadFile(int cardId, const juce::File& file);
    void unloadFile(int cardId);
    void play();
    void pause();
    void seek(double positionSeconds);
    void switchToCard(int cardId);
    double getTotalLength() const;
    double getCurrentPosition() const;
};
```

### Transport 控制

| 元件 | 功能 |
|------|------|
| `◀◀` | 後退 5 秒 |
| `▶/⏸` | 播放/暫停 |
| `▶▶` | 前進 5 秒 |
| 時間顯示 | `01:23/03:45` |
| 進度條 | 可拖曳 seek |

### 鍵盤快捷鍵

- `Space`：播放/暫停
- `←/→`：後退/前進 5 秒
- `1-8`：切換到對應卡牌

### 拖放狀態

| 狀態 | 視覺 |
|------|------|
| 空卡牌 | 淡化卡背 + 虛線框 |
| 拖曳懸停 | 藍色覆蓋層 +「Drop Here」|
| 已載入 | 檔名 + 時長 + [✕] 移除按鈕 |

## 錯誤處理

| 情況 | 處理 |
|------|------|
| 不支援格式 | 卡牌閃紅 + Toast |
| 檔案損壞 | Toast 提示 |
| Sample rate 不一致 | 自動重新取樣 |
| 檔案 >1GB | 拒絕載入 |

## 檔案結構

**新增：**
```
Source/Standalone/
├── StandaloneAudioEngine.h
├── StandaloneAudioEngine.cpp
└── StandaloneHelpers.h

Source/UI/Components/
├── TransportBar.h
├── TransportBar.cpp
├── CardCountControl.h
└── CardCountControl.cpp
```

**修改：**
- PokerCard.h/cpp - 拖放功能
- HeaderBar.h/cpp - 整合 TransportBar
- ControlPanel.h/cpp - 整合 CardCountControl
- BlindCardEditor.h/cpp - 整合引擎
- CMakeLists.txt - 新增檔案

## 實作順序

### Phase 1: 基礎播放
1. StandaloneAudioEngine - 單檔載入與播放
2. TransportBar - 播放/暫停/時間顯示
3. PokerCard 拖放 - 單張卡牌載入音檔

### Phase 2: 完整功能
4. 多音檔同步播放
5. 卡牌切換（無縫切換）
6. 進度條拖曳 seek
7. CardCountControl - 卡牌數量增減

### Phase 3: 整合測試
8. 與現有盲測流程整合
9. 鍵盤快捷鍵
10. 錯誤處理與 Toast
