# BlindCard Standalone Notebook — 設計文件

**狀態**：Design approved（brainstorming 完成）
**範圍**：Standalone 限定（AU/VST3 不做）
**作者**：CEO + Claude（協作 brainstorming）
**日期**：2026-04-27

---

## 0. 願景與一句話定義

> 在 BlindCard standalone 盲測時，給 user 一本「跟著秒數 + 卡 + 輪次走」的聊天式日誌；Reveal 之後自動展開成完整測評報告，可匯出 markdown 給混音師/客戶看。

不是普通筆記本——核心差異化：
1. 每筆筆記隱含 `(playhead 秒數, 當下卡, 當下 round)`
2. 點筆記能 pre-roll 1.5 秒重播當時的耳朵狀態
3. Reveal 是 timeline 上的事件，前後分流可追溯
4. 多輪重洗（reshuffle）內嵌同一 timeline，不是分檔
5. 匯出的 .md 直接是一份測評報告

---

## 1. 設計決策摘要

| # | 主題 | 決策 |
|---|---|---|
| Q1 | 資料模型 | A — Timeline 註記流 `(timestamp, card, round, text)` |
| Q2 | Timestamp 捕獲時機 | B — 第一個 keystroke 凍結 |
| Q3 | UI 形狀 | 聊天彈窗（HybridCompressor chat panel pattern） |
| Q4 | Jump-back 行為 | D — pre-roll 1.5s + 自動播放 |
| Q5 | Reveal 處理 | D — Reveal 是 timeline 上的事件 |
| Q6 | 持久化 | D — Preset embed + SQLite session log + .md export |
| Q7 | 鍵盤模型 | D + B — 智能 focus + Tab + ⌘← / ⌘→ 全域切卡 |
| Q8 | 彈窗開關 | B — Floating window 380×500、`N` hotkey、Header 📓 button |
| Q9 | Reshuffle 行為 | C — 多 round 內嵌同 timeline，round tag |
| Q10 | Edit/delete | D — Soft edit/delete + post-reveal edit 紅色標籤 |
| Q11 | 歷史瀏覽 | B — Header 📚 → modal → read-only timeline + .md 匯出 |

---

## 2. 資料模型 + SQLite Schema

### 2.1 In-memory structs

```cpp
struct Note {
    int64_t       id;                  // SQLite rowid
    int64_t       sessionId;
    int           round;               // 1, 2, 3...
    double        audioTime;           // playhead 秒數，第一個 keystroke 凍結
    int           cardIndex;           // 0..7（A..H）
    juce::String  text;
    int64_t       createdAt;           // unix ms
    int64_t       editedAt;            // 0 = 未編輯
    bool          editedAfterReveal;
    bool          deleted;             // soft delete
};

struct RevealEntry {
    int           round;
    int           cardIndex;
    juce::String  realFilename;
    juce::String  fullPath;
    juce::String  sha1;
    double        durationSec;
};

struct Session {
    int64_t       id;
    int64_t       startedAt;
    int64_t       endedAt;
    int           cardCount;
    juce::String  mode;                // "stars" | "guess" | "qa"
    int64_t       revealedAt;          // 0 = 尚未 reveal
    int64_t       importedFrom;        // 0 = 原生；非 0 = 從 preset 導入
};
```

### 2.2 SQLite schema

```sql
CREATE TABLE sessions (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    started_at    INTEGER NOT NULL,
    ended_at      INTEGER,
    card_count    INTEGER NOT NULL,
    mode          TEXT NOT NULL,
    revealed_at   INTEGER DEFAULT 0,
    imported_from INTEGER DEFAULT 0
);

CREATE TABLE rounds (
    session_id    INTEGER NOT NULL,
    round         INTEGER NOT NULL,
    started_at    INTEGER NOT NULL,
    audio_time    REAL,                -- shuffle 發生時 playhead
    PRIMARY KEY(session_id, round),
    FOREIGN KEY(session_id) REFERENCES sessions(id) ON DELETE CASCADE
);

CREATE TABLE reveal_entries (
    session_id    INTEGER NOT NULL,
    round         INTEGER NOT NULL,
    card_index    INTEGER NOT NULL,
    real_filename TEXT NOT NULL,
    full_path     TEXT NOT NULL,
    sha1          TEXT,
    duration_sec  REAL,
    PRIMARY KEY(session_id, round, card_index),
    FOREIGN KEY(session_id) REFERENCES sessions(id) ON DELETE CASCADE
);

CREATE TABLE notes (
    id                   INTEGER PRIMARY KEY AUTOINCREMENT,
    session_id           INTEGER NOT NULL,
    round                INTEGER NOT NULL,
    audio_time           REAL NOT NULL,
    card_index           INTEGER NOT NULL,
    text                 TEXT NOT NULL,
    created_at           INTEGER NOT NULL,
    edited_at            INTEGER DEFAULT 0,
    edited_after_reveal  INTEGER DEFAULT 0,
    deleted              INTEGER DEFAULT 0,
    FOREIGN KEY(session_id) REFERENCES sessions(id) ON DELETE CASCADE
);

CREATE INDEX idx_notes_session ON notes(session_id, round, audio_time);
CREATE INDEX idx_sessions_started ON sessions(started_at DESC);

-- 預留 v2 用，v1 不建立
-- CREATE VIRTUAL TABLE notes_fts USING fts5(text, content='notes', content_rowid='id');
```

**檔案位置**：`~/Library/Application Support/BlindCard/sessions.sqlite`
**Journal mode**：`PRAGMA journal_mode=WAL`（支援多 instance）

### 2.3 Preset embed

`StandalonePresetManager::PresetData` 擴充：

```cpp
struct PresetData {
    // ... 既有欄位 ...
    juce::ValueTree notebookSnapshot;  // root="Notebook"
};
```

`notebookSnapshot` ValueTree 結構：

```xml
<Notebook sessionId="..." revealedAt="...">
  <Reveals>
    <Entry round="1" cardIndex="0" filename="..." path="..." sha1="..." duration="..."/>
    ...
  </Reveals>
  <Reshuffles>
    <Reshuffle round="2" startedAt="..." audioTime="..."/>
    ...
  </Reshuffles>
  <Notes>
    <Note id="..." round="1" audioTime="42.3" cardIndex="1" text="..."
          createdAt="..." editedAt="..." editedAfterReveal="0" deleted="0"/>
    ...
  </Notes>
</Notebook>
```

載入 preset：永遠新建 sessionId，`sessions.imported_from` 指向原 sessionId（若有）。

---

## 3. UI 結構

### 3.1 Header bar 整合

```
[ Logo  Mode  Shuffle  Reveal ]                    [ 📓 N (3) ]  [ 📚 ]
                                                    ↑ current     ↑ history
```

- `📓 N (3)`：N 是 hotkey 提示、(3) 是當前 session 已寫筆記數（純資訊）
- `📚`：開歷史 sessions modal

### 3.2 NotebookOverlay 解剖（380×500，可 resize，min 320×400）

```
┌─ ⋮⋮  Round 2 · 5 notes ──────────  Show deleted ☐  ✕ ─┐
├──────────────────────────────────────────────────────┤
│  📍 00:42 · Card B (R1)                              │
│     snare 比較有力，但 vocal 太亮                       │
│     ──────────                                       │
│  ─── 🃏 REVEALED at 01:45 ───                        │
│      Card A → Mix v1 - LA-2A                         │
│      Card B → Mix v3 - 1176                          │
│  📍 02:10 · Mix v3 - 1176                            │
│     補充：sibilance 更明顯                             │
│     [edited · post-reveal]                           │
├──────────────────────────────────────────────────────┤
│  📍 captured at 02:35 · Card C  (or "type to start") │
│  ┌────────────────────────────────────────────────┐  │
│  │ Type your note...                              │  │
│  └────────────────────────────────────────────────┘  │
│  Tab: focus card  ⌘← / ⌘→ : switch card  Esc: close │
└──────────────────────────────────────────────────────┘
```

### 3.3 Components（JUCE-native）

| Class | 用途 |
|---|---|
| `NotebookOverlay : juce::Component, juce::ComponentBoundsConstrainer` | 主容器，可拖、resize、Z-order 永遠最上 |
| `NoteListView : juce::Component` | Virtual list，每 row 是 `NoteRowComponent` |
| `NoteRowComponent` | 單條訊息渲染，hover 出 ✏️ ✕ |
| `RevealDividerComponent` | Reveal 對照表分隔符 |
| `ReshuffleDividerComponent` | Round 切換分隔符 |
| `NoteInputBar : juce::Component` | TextEditor + TimestampPreviewLabel + hotkey hint |
| `HistoryModal : juce::DialogWindow` | 歷史 session 列表 + read-only timeline |

### 3.4 視覺主題

- 整體沿用 BlindCard 的 poker / casino 風（深綠 + 金色框 + serif 標題）
- Message list 用中性暗灰背景（避免閱讀疲勞）
- Focus 指示：textarea focus → 邊框 2px 金色；卡 focus → 卡 outline 2px 金色

### 3.5 Empty state

「Press 1–8 or click cards. Start typing to capture a thought at the current moment.」

### 3.6 位置記憶

`ApplicationProperties` 存 `notebook.x / .y / .w / .h`；首次預設 `(screenW - 400, 80)`。

---

## 4. 互動流程

### 4.1 輸入狀態機

```
state: idle ──────────────────────────────────────────────┐
        │ 任何字母/數字/標點 keypress（不含 Tab/Esc/Cmd）  │
        ▼                                                 │
state: capturing                                          │
  • freeze (timestamp = playhead, card = active card)     │
  • TimestampPreviewLabel 變實 ("📍 captured at 02:35")  │
  • textarea 開始接收字                                    │
        │                                                  │
        ├─ Enter             → submit note，state=idle    │
        ├─ Shift+Enter       → newline (留在 capturing)   │
        ├─ Esc               → cancel, clear, state=idle  │
        ├─ 刪到空字串 + 5 秒沒打字 → auto-reset, state=idle │
        └─ 5 秒沒按鍵 + 非空      → 維持 capturing
```

### 4.2 Jump-back 流程

1. 點 message row（任何位置）
2. 記下目前 playhead → push 進 `JumpHistory` stack（最多 10 層）
3. `audioEngine->setTransportPosition(max(0, note.audioTime - 1.5))`
4. `processor.setActiveCard(note.cardIndex)`
5. `audioEngine->play()`
6. Transport 旁出現 `↶ Return to 03:12` 按鈕（5 秒無互動消失，但 stack 保留）
7. 按 `↶` 或 `⌘Z` pop 一層

### 4.3 鍵盤模型（彈窗開時）

| 鍵 | textarea focus | 主畫面 focus | 備註 |
|---|---|---|---|
| 字母/數字/標點 | 打字（首鍵觸發 freeze） | 1–8 切卡 | |
| `Tab` | 推 focus 到主畫面 | `Tab` 又推回 textarea | 一鍵切換 |
| `← / →` | 移動 cursor | 切卡 | textarea 內正常 |
| `⌘← / ⌘→` | 切卡（全域奪鍵） | 切卡 | 永遠切卡 |
| `Space` | 空格 | Transport play/pause | 各管各的 |
| `Esc` | 關彈窗 | 關彈窗 | 全域 |
| `Enter` | submit | （無作用） | |
| `Shift+Enter` | newline | （無作用） | |
| `N` | 普通字元 | toggle 彈窗 | textarea 不奪 |
| `⌘Z` | undo 打字 | 跳回上一 jump-back 位置 | |

**滑鼠**：點任何 card / transport / shuffle button → focus 自動推到主畫面；點 textarea 推回。

### 4.4 Onboarding hint

彈窗底部 hotkey hint 預設顯示，連續開啟 3 次後自動隱藏（`ApplicationProperties.notebook.openCount`）。設定中可重新開啟。

---

## 5. 事件生命週期

### 5.1 Reshuffle 事件

1. 主畫面按 SHUFFLE → `BlindCardEditor` 廣播 `onReshuffle(newRound)`
2. 若 `state == capturing` → in-flight 文字當未送出 draft 保留，timestamp 用 shuffle 前瞬間，submit 時歸**舊 round**
3. timeline 插入 `ReshuffleDividerComponent`：`─── 🔀 RESHUFFLED at 02:15 (Round 2) ───`
4. Toast 1.5 秒：`🔀 Round 2 started, 5 notes from R1 preserved`
5. Header 📓 計數 reset 為當前 round 已寫筆記數
6. SQLite 寫 `rounds` 表新一筆

### 5.2 Reveal 事件

1. 主畫面按 REVEAL → 廣播 `onReveal(revealMap)`
2. `revealMap = {round: {cardIndex: filename}}`
3. timeline 插入 `RevealDividerComponent` 展開所有 round 的對照表
4. 之後新筆記 card 標籤渲染用真名（例：`Card B → Mix v3 - 1176`）
5. **舊筆記資料層不變**（永遠存 cardIndex），渲染層 reveal 後自動 lookup
6. SQLite 更新 `sessions.revealed_at` + 寫 `reveal_entries` 表

### 5.3 Edit lifecycle

1. hover row → ✏️ 出現
2. 點 ✏️ → row 變 inline editor（textarea 取代靜態文字）
3. Enter → 寫回 SQLite，`edited_at = now()`、`edited_after_reveal = (sessions.revealed_at != 0)`
4. 渲染：`edited_at != 0` → `[edited]` 灰標；`edited_after_reveal == 1` → 紅色 `[edited · post-reveal]`
5. Esc → 取消，文字 revert

### 5.4 Delete lifecycle

1. hover row → ✕ 出現
2. 點 ✕ → 立即隱藏（`deleted = 1`），toast：`Note deleted ↶ Undo`（5 秒可 undo）
3. `Show deleted ☐` 勾選後 → deleted row 50% opacity + 刪除線顯示，hover 出 `↻ Restore`
4. 匯出 .md 預設不含 deleted（dialog 有勾選可含）

### 5.5 Race conditions

- Reveal 時 capturing → draft 用 reveal 前 card 標籤儲存（盲評身份保留）；submit 後渲染時若已 reveal，自動套真名
- 多 row hover → ✏️ ✕ 只在最近 hover 的一個顯示
- Jump-back 時 audio 沒載入該檔 → toast `⚠ Audio not loaded`，只切 card 不跳時間

---

## 6. 持久化與匯出

### 6.1 寫入時序（絕不在 audio thread）

```
GUI thread:
  user keystroke → state machine → in-memory model 更新 → UI 重繪
                                 ↓ push WriteOp to lock-free queue

Background thread (SqliteWriter, juce::Thread):
  drain queue (debounce 200 ms) → batch SQL transaction → flush
```

```cpp
struct WriteOp {
    enum class Kind { InsertNote, UpdateNote, DeleteNote, InsertReveal,
                      InsertReshuffle, UpdateSession };
    Kind kind;
    juce::var payload;  // plain data，無 JUCE Component pointer
};
```

### 6.2 Markdown 匯出模板

```markdown
# Blind Test Session — 2026-04-27 16:32:14
- **Cards**: 4
- **Mode**: Stars
- **Duration**: 12:18
- **Total notes**: 12 (3 deleted, hidden)
- **Rounds**: 2

## Round 1

- 📍 `00:42` · Card B — snare 比較有力，但 vocal 太亮
- 📍 `01:30` · Card A — bass 比較緊

## 🔀 Reshuffle at `02:15` → Round 2

- 📍 `00:18` · Card A — 比較糊

## 🃏 Revealed at `03:00`

| Round | Card | Real Track |
| --- | --- | --- |
| 1 | A | Mix v1 - LA-2A.wav |
| 1 | B | Mix v3 - 1176.wav |
| 2 | A | Mix v5 - SSL.wav |
| 2 | B | Mix v1 - LA-2A.wav |

## After Reveal

- 📍 `04:00` · Mix v3 - 1176 — 還是覺得這個最有力 *[edited · post-reveal]*

## Final Ratings (Stars mode)

| Card | Real Track | Stars |
| --- | --- | --- |
| A (R1) | Mix v1 - LA-2A | ⭐⭐⭐⭐ |
| B (R1) | Mix v3 - 1176 | ⭐⭐⭐ |
```

**檔名**：`BlindCard_Session_2026-04-27_1632.md`
**預設位置**：`~/Documents/BlindCard/Exports/`

### 6.3 大小管理

- DB 不主動清理
- 超過 100 MB 時 startup 警告 + 提供「Archive sessions older than ___」按鈕（v1 只警告不自動刪）

---

## 7. 歷史 Session 瀏覽

### 7.1 進入點

Header 📚 button → `HistoryModal`（640×480，置中，✕ / Esc 關閉）

### 7.2 列表 layout

```
┌─ Session History ────────────────────────────  Sort: ▼ Newest first  ✕ ┐
├──────────────────────────────────────────────────────────────────────┤
│ Date / Time          Cards  Mode    Notes  Duration  Rounds  Actions │
├──────────────────────────────────────────────────────────────────────┤
│ 2026-04-27  16:32     4    Stars     12     12:18      2     ▶  ⋮  │
│ 2026-04-27  14:10     3    Guess      5     08:45      1     ▶  ⋮  │
│ 2026-04-26  22:05     8    Stars     38     34:12      4     ▶  ⋮  │
└──────────────────────────────────────────────────────────────────────┘
```

- `▶`：開 read-only timeline view（取代列表，左上 ◀ Back）
- `⋮`：menu — Export as Markdown / Export as JSON / Delete session
- 排序選項：Newest first / Oldest first / Most notes / Longest duration

### 7.3 Read-only timeline

跟 `NoteListView` 共用，差別：
- `readOnly = true` → hover 不出 ✏️ ✕
- 點 row 不 jump audio（v1 沒 reload，無 audio engine）
- 顯示 `Show deleted ☐` 切換
- Hotkey hint：「`E` Export · `D` Delete session · `Esc` Back」

### 7.4 Delete session

⋮ → Delete → 確認 dialog：「Permanently delete this session and all 12 notes? This cannot be undone.」→ SQLite **真刪**（不是 soft delete）。

### 7.5 Empty state

「No history yet. Your sessions will be auto-saved here.」

### 7.6 v2 路線圖（不在本次範圍）

- **FTS5 全文搜尋**：建 `notes_fts` virtual table、搜筆記內文 + 檔名
- **Reload to main editor**：用 `reveal_entries.full_path + sha1` 驗證所有檔在原位 → 還原到 PokerTable + 切到該 session（彈窗變 read-write、新筆記歸 round=N+1）
- **檔案不見的部分還原**：`3 of 8 audio files missing, locate them?` + 逐一 file picker

---

## 8. 邊界處理

| # | 情境 | 處理 |
|---|---|---|
| 1 | SQLite 寫失敗（disk full / permission） | Queue 累積、in-memory 仍正常；status bar `⚠ Notebook autosave failed`；10s 重試 × 3；失敗跳 alert 提供「Export .json 救資料」 |
| 2 | DB 損壞 | 啟動時 `PRAGMA integrity_check` 失敗 → rename `sessions.sqlite.corrupt-<ts>` + 開新 DB + toast |
| 3 | Audio file 中途被移動 | jump-back 失敗 toast `⚠ Audio not playable`；筆記不受影響 |
| 4 | 0 cards 開彈窗 | placeholder `Load audio first to start taking notes`，keystroke 不 freeze |
| 5 | Audio 暫停中打字 | 仍 freeze，訊息加 `⏸` 小 icon |
| 6 | 同秒多筆訊息 | 允許，用 `created_at`（millisecond）排序 |
| 7 | Card index 超出當前 cards 數 | `Card B (n/a)` 灰色 + tooltip |
| 8 | Preset 匯入 ID 衝突 | 永遠新建 session ID + `imported_from` 紀錄 |
| 9 | 系統時鐘改變（NTP） | `audioTime` 用 audio engine 內部時間，不受影響 |
| 10 | 多 instance 同時開 | WAL mode 支援；UI 不互相同步 |
| 11 | 主視窗縮極小 | `ComponentBoundsConstrainer` 確保不超 desktop；自動 dock 到 desktop |

---

## 9. 測試策略

### 9.1 Unit tests（`Source/Tests/NotebookTests.cpp`，JUCE UnitTest）

- `NotebookStoreTest`：CRUD 正確性、soft delete 後 query 排除、edit 後 `edited_at` 更新
- `SqliteWriterTest`：debounce 200 ms 行為、queue 滿時不 block GUI、重試邏輯
- `MarkdownExporterTest`：固定 fixture session → byte-exact match 預期 `.md`
- `PresetSerializationTest`：`notebookSnapshot` ↔ `ValueTree` round-trip

### 9.2 Integration tests

- 完整流程：load 4 cards → 寫 3 筆 → shuffle → 寫 2 筆 → reveal → edit 1 筆 → export → diff
- 載入 preset → 驗證 notebook 還原 + sessions 表新增 imported

### 9.3 Manual smoke tests（發布前必跑）

| # | 名稱 | 步驟 |
|---|---|---|
| L1 | Capture flow | 開窗 → 打 5 字 → Enter → 訊息出現 + DB 有資料 |
| L2 | Jump-back | 點 3 條 → pre-roll 1.5s 跳轉 + 自動播放 |
| L3 | Reshuffle in-flight draft | 打到一半 SHUFFLE → draft 歸舊 round + divider 出現 |
| L4 | Reveal lifecycle | reveal 前寫 + reveal + reveal 後 edit → 紅標 |
| L5 | Keyboard model | textarea focus 時 ⌘← 切卡 work；Tab 推 focus 卡 outline 出現 |
| L6 | Crash recovery | 寫 5 筆 → kill -9 → 重開 → 訊息全在 |
| L7 | Cross-session history | 寫 3 個 session → 開歷史 modal → 點 → read-only timeline 正確 |
| L8 | Disk full simulation | 填滿 disk → 寫筆記 → fallback 警告 + in-memory 正常 |

### 9.4 Regression（每次發布前）

L1–L8 全跑 + 既有 BlindCard PokerTable / shuffle / reveal smoke tests 全跑（確認 notebook 沒打壞主流程）。

---

## 10. 範圍切割（v1 / v2）

### v1（本設計）

- ✅ 所有 §2–§8 功能
- ✅ Markdown + JSON 匯出
- ✅ 歷史 session read-only 瀏覽
- ✅ Preset embed

### v2（未來）

- ⏸ FTS5 全文搜尋
- ⏸ Reload history session to main editor（含檔案不見的還原 flow）
- ⏸ 跨 session 統計（「我這個月寫了幾條筆記」「我最常評為 5 星的混音師」）
- ⏸ 匯入到 SugoiTeamChat 知識庫
- ⏸ 自動 archive 老 session（>1 年）

---

## 11. 實作建議分層

實作順序（適合開新 worktree 漸進開發）：

1. **Layer 1 — Data layer**：`NotebookStore` + `SqliteWriter` + schema migration + 所有 unit tests
2. **Layer 2 — Renderer**：`NoteListView` + `NoteRowComponent` + 兩種 divider，先用 hardcoded fixture 渲染
3. **Layer 3 — Input + state machine**：`NoteInputBar` + first-keystroke freeze + submit
4. **Layer 4 — Overlay + window management**：`NotebookOverlay` 拖移、resize、位置記憶
5. **Layer 5 — Header integration**：📓 / 📚 button + hotkey N + counter
6. **Layer 6 — Event integration**：reshuffle / reveal 廣播接收 + dividers 插入
7. **Layer 7 — Edit/delete**：✏️ ✕ + soft delete + post-reveal 紅標
8. **Layer 8 — Jump-back**：pre-roll + jump history stack + `↶` 鈕
9. **Layer 9 — Keyboard model**：focus 切換、⌘← / ⌘→ 全域、Tab、Esc
10. **Layer 10 — Persistence**：preset embed + load
11. **Layer 11 — Markdown 匯出**：模板 + dialog
12. **Layer 12 — History modal**：列表 + read-only view + delete session
13. **Layer 13 — Edge cases + smoke tests L1–L8**

每層完成後跑該層相關 unit/integration tests + 該層適用的 smoke tests。

---

## 12. 風險與未解問題

| 風險 | 評估 | 緩解 |
|---|---|---|
| 第一個 keystroke 觸發 freeze 跟 BlindCard 主畫面數字鍵切卡衝突 | 中等 — 彈窗開時主畫面不收數字鍵（focus 已被 textarea 拿走），但 `Tab` 推回主畫面後數字鍵又活了 → 若那時打 `1` 是想切卡還是想回筆記？ | 用 focus 視覺指示明確區分；`Tab` 推到主畫面後字母/數字鍵不再 freeze 筆記，需再 `Tab` 回 textarea 才行 |
| SQLite 寫頻率高（每個 keystroke 都觸發？）| 低 — 只 submit 才寫；keystroke 只更新 in-memory | 已有 200 ms debounce 保險 |
| ⌘← / ⌘→ 在 textarea 是「行首/行尾」標準快捷鍵，奪走會違反 macOS HIG | 中等 — 有些 user 會被搞混 | 在 hotkey hint 明確標示；保留 `⌥← / ⌥→` 給 textarea 行內導航（macOS 標準） |
| 多 round 在同一 timeline 的視覺密度 | 中等 — 跑到 round 5+ 時 timeline 很擠 | round tag 用淡灰小字；divider 用薄線；列表 virtual scroll 確保效能 |
| 匯出的 markdown 在不同 markdown viewer 渲染不一致 | 低 | 用最 portable 子集（GFM core），不用擴充語法 |

---

## 13. 變更紀錄

| 日期 | 動作 | 人員 |
|---|---|---|
| 2026-04-27 | Brainstorming 完成、設計定稿 | CEO + Claude |
