# BlindCard User Manual

**Version 1.0.0 | Sugoi Audio**

---

## Table of Contents

1. [What is BlindCard?](#1-what-is-blindcard)
2. [Installation](#2-installation)
3. [Getting Started](#3-getting-started)
4. [Interface Overview](#4-interface-overview)
5. [Plugin Mode (DAW)](#5-plugin-mode-daw)
6. [Standalone Mode](#6-standalone-mode)
7. [Rating Modes](#7-rating-modes)
8. [Level Matching](#8-level-matching)
9. [Keyboard Shortcuts](#9-keyboard-shortcuts)
10. [Settings](#10-settings)
11. [FAQ & Troubleshooting](#11-faq--troubleshooting)

---

## 1. What is BlindCard?

BlindCard is a blind A/B testing plugin for audio professionals. It lets you compare up to 8 audio tracks without knowing which is which, eliminating bias from your listening decisions.

**Use cases:**

- Compare mix revisions without knowing which version you're hearing
- Evaluate competing plugins on the same source material
- Train your ears to identify EQ, compression, and other processing
- Settle debates about audible differences with objective blind tests

BlindCard supports three evaluation modes (Stars, Guess, Q&A), automatic level matching, and multi-round testing. It runs as an AU plugin, VST3 plugin, or standalone application.

---

## 2. Installation

### macOS

Run **BlindCard-1.0.0-Installer.pkg** and choose which formats to install:

| Format | Install Location | Use With |
|--------|-----------------|----------|
| AU (Audio Unit) | `/Library/Audio/Plug-Ins/Components/` | Logic Pro, GarageBand |
| VST3 | `/Library/Audio/Plug-Ins/VST3/` | Ableton Live, Cubase, Studio One, Reaper |
| Standalone | `/Applications/` | No DAW required |

### Windows

Run **BlindCard-1.0.0-Windows-x64-Setup.exe** and follow the installer prompts.

After installation, restart your DAW to scan for the new plugin.

---

## 3. Getting Started

### Quick Start (Plugin Mode)

1. Insert BlindCard on **each track** you want to compare (2-8 tracks).
2. All instances share the same game state automatically.
3. Click **SHUFFLE** to start blind testing.
4. Click cards to switch between tracks and evaluate them.
5. Click **REVEAL** when done to see the results.

### Quick Start (Standalone Mode)

1. Open BlindCard from your Applications folder.
2. Use the **+/-** buttons to set the number of cards (2-8).
3. Drag and drop audio files onto each card, or click **Import** to batch load.
4. Click **SHUFFLE** and use the transport bar to play audio.
5. Click cards to switch tracks. Click **REVEAL** when done.

---

## 4. Interface Overview

```
+----------------------------------------------------------+
| [Logo]  BlindCard        [Track Info]    [?] [theme] [gear]  <- Header Bar
+----------------------------------------------------------+
|              [ Stars ]  [ Guess ]  [ Q&A ]               <- Mode Selector
+----------------------------------------------------------+
|                                                          |
|           [Card 1]  [Card 2]  [Card 3]  [Card 4]        <- Poker Table
|                                                          |
+-----------------------------+----------------------------+
|  Cards: 4    Rounds: 1/3   |         Results             |
|  Level-Match: ON           |    Card 1: ★★★★☆           <- Control & Results
|  [SHUFFLE] [REVEAL] [RESET]|    Card 2: ★★★☆☆           |
+-----------------------------+----------------------------+
```

### Header Bar

- **Logo & Branding** — Left side
- **Track Info Capsule** — Center: shows "NOW PLAYING," current track name, and RMS level
- **? Button** — Opens the Quick Start Guide
- **Theme Toggle** — Switches between dark and light mode
- **Gear Button** — Opens Settings (a green dot appears when an update is available)

### Mode Selector

Three rating modes: **Stars**, **Guess**, and **Q&A**. The selector locks during blind testing to prevent mode changes mid-test.

### Poker Table

The main area displays 2-8 cards in a casino poker layout:
- **1-4 cards** — Single row
- **5-8 cards** — Two rows (4 per row)

Each card is styled as a poker card. During blind testing, cards show their back design. When revealed, cards flip to show both the poker face and the real track name.

### Control Panel (Bottom Left)

- **Cards** — Number of active tracks
- **Rounds** — Slider to set round count (1-8)
- **Level-Match** — Toggle for automatic level compensation
- **Action Buttons** — SHUFFLE, REVEAL, NEXT, RESET

### Results Panel (Bottom Right)

Displays ratings, scores, or Q&A results depending on the active mode.

---

## 5. Plugin Mode (DAW)

### Setup

1. Create the tracks you want to compare in your DAW.
2. Insert BlindCard as an effect on each track.
3. All BlindCard instances automatically detect each other and share game state.
4. Track names from your DAW are used as card names.

### How It Works

- **Solo behavior** — Only the selected card passes audio. All other instances are muted with a smooth 10ms fade to prevent clicks.
- **Track detection** — BlindCard reads the track name from your DAW automatically.
- **Maximum tracks** — 8 simultaneous instances.
- **Adding tracks** — New instances can join during Setup phase. Once shuffled, no new tracks can be added.
- **Removing tracks** — If you remove a plugin instance during a test, its card is marked as removed but the test continues.

### Supported DAWs

BlindCard works with any DAW that supports AU or VST3:
- Logic Pro
- Ableton Live
- Cubase / Nuendo
- Studio One
- Reaper
- FL Studio
- Pro Tools (VST3)

---

## 6. Standalone Mode

Standalone mode lets you run blind tests without a DAW by loading audio files directly.

### Loading Audio Files

**Drag & Drop** — Drag audio files directly onto individual cards.

**Import Button** — Click "Import" to open a file browser. Select multiple files to batch load them into consecutive card slots. Files are sorted alphabetically.

**Supported formats:** WAV, MP3, AIFF (max 1 GB per file).

### Transport Controls

The transport bar appears in the header area:

- **Play/Pause** — Toggle playback (or press Spacebar)
- **Skip Backward** — Jump back 5 seconds
- **Skip Forward** — Jump forward 5 seconds
- **Progress Bar** — Click or drag to seek
- **Time Display** — Shows current position / total duration

All cards share the same playback position for fair comparison.

### Card Count

Use the **+/-** buttons to adjust the number of cards from 2 to 8. In plugin mode, this is determined automatically by the number of DAW tracks.

### Presets

- **Save** — Save the current card configuration (file paths and card count) as a preset.
- **Load** — Select a saved preset from the dropdown.
- **Delete** — Remove the selected preset.

Presets let you quickly recall test setups you use frequently.

---

## 7. Rating Modes

### Stars Mode

Rate each track on a 1-5 star scale.

**How to use:**
1. Click a card to listen to it.
2. Click the star buttons below the card to rate it (1-5).
3. Repeat for all cards.
4. Click REVEAL to see rankings.

**Results:** Cards are sorted by average rating (highest first). Top 3 cards receive gold, silver, and bronze medals.

**Multi-round:** Each round records independent ratings. The final result shows the average across all rounds.

### Guess Mode

Identify which original track each card corresponds to.

**How to use:**
1. Click each card to listen.
2. Use the dropdown below each card to select which track you think it is.
3. Click **Submit Guesses** in the Results Panel to lock in your answers.
4. Click REVEAL to check your accuracy.

**Results:** Each guess shows a checkmark (correct) or cross (wrong) with the actual track name. A final score is displayed as "X/Y (Z%)."

**Multi-round:** After reveal, there is a 3-second countdown before auto-advancing to the next round. Scores are cumulative across rounds.

### Q&A Mode

Answer targeted questions about specific tracks.

**How to use:**
1. A question appears at the top: "Which card is [Track Name]?"
2. Click the card you think matches.
3. A 3-second countdown reveals whether you were correct.
4. The next question appears automatically.

**Configuration:** Set the number of questions (1-8) using the Questions slider in the Control Panel.

**Results:** Each question's result is shown, along with a final score percentage.

---

## 8. Level Matching

Volume differences bias listening tests. A louder track often sounds "better" even when it isn't. Level Matching automatically compensates for this.

### How It Works

1. Toggle **Level-Match** to ON in the Control Panel.
2. BlindCard measures the loudness (LUFS) of each track over a 10-second window.
3. Status indicators show progress:
   - **Waiting** — Measurement hasn't started
   - **Detecting...** — Measurement in progress (with countdown)
   - **Ready** — Calibration complete
4. Each track's gain is automatically adjusted so all tracks play at the same perceived volume.

### Technical Details

- Measurement uses integrated LUFS over a 10-second window.
- The median LUFS value is used as the reference level.
- Auto gain per track = median LUFS - track LUFS (clamped to +/-24 dB).
- Manual gain trim is also available per card (+/-12 dB).

### Tips

- Ensure all tracks are playing audio during the measurement window for accurate results.
- Level matching is available in both Setup and BlindTesting phases.
- After a Reset, calibration is restarted automatically if Level-Match is still enabled.

---

## 9. Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| **1-8** | Select card by position |
| **Tab** | Next card |
| **Shift+Tab** | Previous card |
| **[ (left bracket)** | Previous card |
| **] (right bracket)** | Next card |
| **Left / Right Arrow** | Previous / Next card |
| **Up / Down Arrow** | Navigate between rows (5-8 card layouts) |
| **Spacebar** | Play / Pause (Standalone mode only) |

**Note:** In some DAWs (e.g., Logic Pro), arrow keys may be intercepted by the host. Use number keys, Tab, or bracket keys for reliable navigation.

**Tip:** Click anywhere in the plugin window first to ensure BlindCard has keyboard focus.

---

## 10. Settings

Click the **gear icon** in the top-right corner to open Settings.

### Language

Choose from 5 languages:
- English
- Traditional Chinese (繁體中文)
- Simplified Chinese (简体中文)
- Japanese (日本語)
- Korean (한국어)

### Version & Updates

The Settings panel displays the current version number. If a newer version is available, a green notification dot appears on the gear icon and the Settings panel shows a download link.

### Theme

Toggle between dark and light mode using the sun/moon icon in the header bar. Dark mode is recommended for typical studio environments.

---

## 11. FAQ & Troubleshooting

**Q: The plugin doesn't appear in my DAW.**
A: Restart your DAW after installation. If using Logic Pro, you may need to open Audio Units Manager and rescan.

**Q: Can I add more tracks after shuffling?**
A: No. All tracks must be set up before clicking SHUFFLE. Click RESET to return to Setup and add more tracks.

**Q: Why do all cards sound the same volume?**
A: Level-Match is enabled. This is intentional — it ensures fair comparison by normalizing loudness. Disable it in the Control Panel if you want to hear original levels.

**Q: The keyboard shortcuts don't work.**
A: Click anywhere inside the plugin window first to give it keyboard focus. Some DAWs intercept certain keys; use number keys (1-8) or bracket keys ([ ]) which work reliably across all DAWs.

**Q: Can I use BlindCard for video?**
A: BlindCard is audio-only. It does not process or sync with video.

**Q: What is the maximum number of tracks?**
A: 8 tracks (cards) per test session.

**Q: How many rounds can I do?**
A: Up to 8 rounds per session. Each round re-shuffles the card positions.

**Q: Does BlindCard add latency?**
A: BlindCard reports zero latency. The mute/unmute switching uses a 10ms fade for click-free transitions, but this does not add processing latency.

---

## System Requirements

- **macOS:** 10.13 (High Sierra) or later
- **Windows:** Windows 10 x64 or later
- **Formats:** AU, VST3, Standalone
- **Display:** Minimum 900x750 resolution

---

## Support

- **Website:** [sugoiaudio.com](https://sugoiaudio.com)
- **GitHub:** [github.com/SugoiAudioTech/BlindCard](https://github.com/SugoiAudioTech/BlindCard)

---

*BlindCard is free and open-source software, licensed under AGPL-3.0. No license key required.*
