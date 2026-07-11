## ADDED Requirements

### Requirement: AAX format is enabled in the build configuration
The build SHALL configure the AAX SDK path and include AAX in the plugin's output formats, with a valid AAX identifier and category, so that a Pro Tools `.aaxplugin` bundle is produced.

#### Scenario: CMake enables AAX with correct metadata
- **WHEN** `CMakeLists.txt` is configured
- **THEN** the AAX SDK path is set via `juce_set_aax_sdk_path` guarded by an existence check on `${AAX_SDK_PATH}/Interfaces/AAX.h`
- **AND** `juce_add_plugin` lists `AAX` in `FORMATS`
- **AND** `AAX_IDENTIFIER` is `com.sugoi.blindcard` and `AAX_CATEGORY` is a JUCE-supported category (`AAX_ePlugInCategory_None`)

#### Scenario: AAX config is committed to source control
- **WHEN** the change is implemented
- **THEN** the CMake AAX configuration is committed (no longer an uncommitted working-tree change)

### Requirement: AAX builds as a universal binary
The AAX bundle SHALL be built as a macOS universal binary containing both `arm64` and `x86_64` slices so it loads natively on Apple Silicon and Intel Pro Tools.

#### Scenario: Clean rebuild produces a universal .aaxplugin
- **WHEN** a clean universal build is run
- **THEN** `<build>/BlindCard_artefacts/Release/AAX/Blind Card.aaxplugin` exists
- **AND** `lipo -archs` on its Mach-O binary reports both `arm64` and `x86_64`

### Requirement: AAX build passes AAX Validator core tests
The built `.aaxplugin` SHALL pass the AAX Validator core test suite, confirming Describe configuration, load/unload stability, and parameter behavior.

#### Scenario: Core validator tests pass
- **WHEN** AAX Validator runs against the installed `.aaxplugin`
- **THEN** `test.describe_validation`, `test.load_unload`, `test.parameters`, `test.parameter_traversal.linear`, and `test.parameter_traversal.random` report `E_COMPLETED_PASS`

#### Scenario: Known-ignorable results are documented, not treated as failures
- **WHEN** the validator reports `E_ABORTED` on info/SysInfo-dependent probes or `FAIL` on `test.page_table.load` (no Page Table XML)
- **THEN** these are recorded as known-ignorable (tool/environment issues), and do not block release
