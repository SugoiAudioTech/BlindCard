# AAX Validator Run Notes — BlindCard

Date: 2026-07-12
Plugin: `/Library/Application Support/Avid/Audio/Plug-Ins/Blind Card.aaxplugin`
Validator: `aax-validator-dsh-2024-6-0-138bab0d-mac-arm64` (DigiShell v24.9.0x14)

## Result

**Blocked by a known local tool/environment bug — NOT a plugin defect.**

Both `runtests` (full suite) and single `runtest [test.describe_validation, ...]`
return `result_status: E_ABORTED`. Root cause is the validator's bundled Ruby
script crashing before it can drive the plugin:

```
.../Tools/DTT/sources/classes/SysInfo.rb:249:in `block in load_body':
  undefined method `[]' for nil:NilClass (NoMethodError)
```

`SysInfo.rb` parses `system_profiler` output whose format changed on this macOS
version, so `DTTMonitor.new → SysInfo` throws and every test aborts. A residual
socket from a crashed run also surfaces as `ServeTests error: bind: Address
already in use` until stale processes are killed.

## Why this is known-ignorable

This is the identical state SugoiDimension shipped its AAX in (see the
`sugoi-audio-aax-protools` skill per-plugin table: "Validator 因本機 SysInfo.rb
bug 全 abort（工具問題非插件）"). The skill's "已知可忽略的問題" section lists
these SysInfo-dependent aborts explicitly.

The bundle itself is well-formed: universal (`x86_64 arm64`), no nested dylibs,
JUCE-generated AAX Describe. The Describe config is produced by JUCE's AAX
wrapper at build time.

## Real validation gate

Functional validation moves to **Pro Tools Developer** (task 6): load in the
Insert menu, audio passes, UI renders, controls respond, bypass works across
buffer sizes. This is the acceptance test that actually exercises the plugin.

## Follow-up (optional, not blocking)

If a machine with a compatible `system_profiler`/Ruby is available (or the Avid
validator is patched), re-run `runtests` there to capture PASS records for
`describe_validation`, `load_unload`, `parameters`, `parameter_traversal.*`.
