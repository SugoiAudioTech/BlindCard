## Why

BlindCard currently ships AU, VST3, and Standalone formats but has no Pro Tools support. AAX is already partially wired in (CMake `FORMATS` includes AAX and the SDK path is set — uncommitted), and a dev build exists, but it is only ad-hoc signed, has not passed AAX Validator, is not PACE-signed, and is absent from the installer pipeline. Pro Tools users cannot load BlindCard today. This change takes AAX from "compiles" to "release-grade": validated, PACE-signed, notarized, and bundled into the macOS installer alongside the other three formats.

## What Changes

- Finalize and commit the CMake AAX configuration (SDK path, `FORMATS ... AAX`, `AAX_IDENTIFIER`, `AAX_CATEGORY`).
- Clean universal (arm64 + x86_64) rebuild and confirm the `.aaxplugin` bundle is produced.
- Verify the AAX build with AAX Validator — core tests (`describe_validation`, `load_unload`, `parameters`, `parameter_traversal.*`) must PASS; document known-ignorable ABORTs.
- Extend `packaging/sign-and-package.sh` to sign the AAX bundle with PACE Eden `wraptool` (with `--extrasigningoptions` for hardened runtime + timestamp + entitlements), add pre-flight checks (wraptool present, iLok connected, `.aaxplugin` exists), build an `BlindCard-AAX.pkg`, and install it to `/Library/Application Support/Avid/Audio/Plug-Ins/`.
- Extend `packaging/Distribution/distribution.xml` to add an AAX choice + `pkg-ref`.
- Notarize the whole installer (AAX included) via the existing `notarize.sh`.
- One-time PACE Central setup (manual, user-performed): create BlindCard Product + Wrap Configuration to obtain a Wrap GUID; record it in memory + the AAX skill's per-plugin table.
- Pro Tools Developer load/audio/UI test as an acceptance gate.

## Capabilities

### New Capabilities
- `aax-build`: BlindCard compiles to a universal `.aaxplugin` bundle via the JUCE + AAX SDK toolchain, and passes AAX Validator core tests.
- `aax-packaging`: The AAX bundle is PACE-signed (hardened-runtime + timestamp), packaged into the distribution installer, and Apple-notarized alongside AU/VST3/Standalone.

### Modified Capabilities
<!-- No existing OpenSpec specs in this repo (freshly initialized); all capabilities are new. -->

## Impact

- **Build**: `CMakeLists.txt` (AAX config — already drafted, uncommitted).
- **Packaging**: `packaging/sign-and-package.sh`, `packaging/Distribution/distribution.xml`; relies on existing `packaging/notarize.sh` and `packaging/BlindCard.entitlements`.
- **Tooling / external**: AAX SDK 2.9.0, AAX Validator (DigiShell), PACE Eden `wraptool`, iLok (must be connected), PACE Central account (manual Product/Wrap Config setup).
- **Install target**: new path `/Library/Application Support/Avid/Audio/Plug-Ins/` (distinct from the AU/VST3/Standalone targets).
- **Docs/memory**: `pace-signing-credentials.md` and `sugoi-audio-aax-protools` skill per-plugin table gain a BlindCard row.
- **Non-goals**: no DSP/UI/behavior changes; no Page Table XML (S6 hardware mapping) this round; no Windows AAX build.
