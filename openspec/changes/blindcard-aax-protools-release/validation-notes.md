# AAX Validator Run Notes — BlindCard

Date: 2026-07-12
Plugin: `/Library/Application Support/Avid/Audio/Plug-Ins/Blind Card.aaxplugin`
Validator: `aax-validator-dsh-2024-6-0-138bab0d-mac-arm64` (DigiShell v24.9.0x14)

## Result — PASS

All 5 core tests pass, 0 FAIL, 0 ABORT:

| Test | Status |
|------|--------|
| `test.describe_validation` | `E_COMPLETED_PASS` |
| `test.load_unload` | `E_COMPLETED_PASS` |
| `test.parameters` | `E_COMPLETED_PASS` |
| `test.parameter_traversal.linear` | `E_COMPLETED_PASS` |
| `test.parameter_traversal.random` | `E_COMPLETED_PASS` |

Non-blocking warnings in `describe_validation` (code `-14001`):
- "Plug-in category for the effect is AAX_ePlugInCategory_None." — expected;
  BlindCard is a quiz/utility plugin, `None` is the correct category.
- "Algorithm context contains gaps between registered fields." (×8) — standard
  JUCE AAX wrapper warning about the algorithm context layout; a warning, not a
  failure. The stage still reports PASSED.

## Root cause of the initial all-ABORT (fixed)

First run had every test `E_ABORTED`. Root cause was NOT the plugin — it was the
validator's bundled Ruby crashing before it could drive the plugin:

```
DTT/sources/classes/SysInfo.rb:249:in `block in load_body':
  undefined method `[]' for nil:NilClass (NoMethodError)
```

`SysInfo.rb` parses `diskutil` output to build a disk overview. On current macOS,
some volume lacks the `Container Free Space` / `Container Available Space` keys,
so `disk['Container Free Space']` is `nil`, and the unguarded
`nil[/regex/]` slice throws. This crashes `DTTMonitor` → every test aborts. This
is the same machine-wide bug that made SugoiDimension's AAX "全 abort" on
2026-07-11.

## Fix applied to the validator tool (local, backed up)

Patched `SysInfo.rb` lines 249 & 251 to nil-guard the regex slice
(`.orig` backup kept alongside):

```ruby
# before
disk['Container Free Space'] = disk['Container Free Space'][/^(\d*\.?\d*\s\w+)/]
disk['Total Size']           = disk['Total Size'][/^(\d*\.?\d*\s\w+)/]
# after
disk['Container Free Space'] = disk['Container Free Space'] && disk['Container Free Space'][/^(\d*\.?\d*\s\w+)/]
disk['Total Size']           = disk['Total Size'] && disk['Total Size'][/^(\d*\.?\d*\s\w+)/]
```

This is a workaround for an Avid-tool regression under newer macOS `diskutil`
output, not a plugin change.

## How to run (avoid the Cycle_Counts crash)

Full `runtests` still dies in `DSH_AAXVAL_Cycle_Counts` with a "Broken pipe"
(that CPU-cycle harness's own issue) and aborts the remaining suite. Run the
core tests in a single dsh session with individual `runtest [...]` commands to
avoid it (rapid separate `dsh` invocations hit `bind: Address already in use`
from the prior socket's TIME_WAIT):

```
load_dish aaxval
runtest [test.describe_validation, "<plugin>"]
runtest [test.load_unload, "<plugin>"]
runtest [test.parameters, "<plugin>"]
runtest [test.parameter_traversal.linear, "<plugin>"]
runtest [test.parameter_traversal.random, "<plugin>"]
quit
```

## Remaining gate

Pro Tools Developer load/audio/UI acceptance (task 6) is still the end-to-end
functional check.
