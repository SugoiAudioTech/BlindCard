# Blind Card

Blind audio comparison tool with a poker card themed UI. Supports AU/VST3 plugins and standalone application.

## Features

- **Blind Testing Mode**: Random shuffle eliminates bias for objective audio comparison
- **Multiple Rating Methods**:
  - Star rating (1-5 stars)
  - Guess mode (guess the original track)
  - Q&A quiz mode
- **Loudness Matching**: Automatic LUFS measurement and gain compensation
- **Poker Card UI**: Casino-style interface, supports up to 8 cards
- **Standalone Mode**: Built-in audio player, can load audio files for direct comparison

## System Requirements

- macOS 10.15+
- CMake 3.22+
- C++17 compiler

## Build

```bash
git clone https://github.com/SugoiAudioTech/BlindCard.git
cd BlindCard
mkdir build && cd build
cmake ..
cmake --build .
```

JUCE is automatically downloaded during the CMake configure step.

After building, plugins are automatically copied to system plugin directories.

## Formats

- AU (Audio Unit)
- VST3
- Standalone

## Usage

Blind Card lets you compare audio tracks without knowing which is which — eliminating bias from your decisions.

### As a Plugin (AU/VST3)

1. **Insert Blind Card** on each track you want to compare (2-8 tracks)

2. **Choose a rating mode**:
   - ⭐ **Stars** – Rate each track from 1-5 stars
   - 🎯 **Guess** – Identify which plugin/mix is on each card
   - ❓ **Q&A** – Find a specific track when prompted

3. **Click "SHUFFLE"** to randomize and hide track identities

4. **Click cards to listen and rate** (or use ← → arrow keys)

5. **Click "REVEAL"** to see results and actual track names

6. **Click "RESET"** to start a new test

### As a Standalone App

1. Launch the standalone application
2. Drag & drop or import audio files to compare
3. Follow the same workflow as above

### Tips

💡 **Level Matching**: Enable "Auto Gain" to level-match all tracks, ensuring fair comparison without volume bias.

💡 **Keyboard Shortcuts**: Use arrow keys (← →) to quickly switch between cards during testing.

## Project Structure

```
Source/
├── Core/                 # Core logic
│   ├── BlindCardManager  # Game state management
│   └── Types.h           # Data structure definitions
├── Standalone/           # Standalone mode audio engine
├── UI/
│   ├── Components/       # UI components
│   ├── Theme/            # Theme and fonts
│   └── Animation/        # Animation system
└── Plugin*.cpp/h         # JUCE plugin entry
```

## License

Copyright (c) 2026 Sugoi Audio Tech

[AGPL-3.0](https://www.gnu.org/licenses/agpl-3.0.html) - See [LICENSE](LICENSE) for details.

This project is based on [JUCE](https://juce.com/), which is licensed under AGPL-3.0. As a derivative work, this project must also be licensed under AGPL-3.0.
