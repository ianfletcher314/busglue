# BusGlue Testing Specification

This document defines comprehensive test specifications for the BusGlue audio plugin, designed to catch issues before they become problems in production environments.

---

## Table of Contents

1. [DSP Unit Tests](#1-dsp-unit-tests)
2. [UI Tests](#2-ui-tests)
3. [Integration Tests](#3-integration-tests)
4. [Regression Tests](#4-regression-tests)

---

## 1. DSP Unit Tests

### 1.1 Envelope Follower Tests

#### TEST-DSP-ENV-001: Peak Detection Accuracy
- **What it tests:** EnvelopeFollower correctly detects peak levels in Peak mode
- **Expected behavior:** Envelope output should match absolute peak values within 0.1dB for steady-state signals
- **How to verify:**
  1. Generate 1kHz sine wave at known amplitude (e.g., -6dB)
  2. Process through EnvelopeFollower in Peak mode
  3. After settling, verify envelope matches expected peak level
- **Priority:** Critical

#### TEST-DSP-ENV-002: RMS Detection Accuracy
- **What it tests:** EnvelopeFollower correctly computes RMS levels
- **Expected behavior:** RMS of sine wave should be ~3dB lower than peak (0.707 ratio)
- **How to verify:**
  1. Generate 1kHz sine wave at 0dB peak
  2. Process through EnvelopeFollower in RMS mode
  3. Verify output is approximately -3.01dB
- **Priority:** Critical

#### TEST-DSP-ENV-003: Attack Time Accuracy
- **What it tests:** Attack time constants are correctly implemented
- **Expected behavior:** Envelope should reach 63.2% of target within specified attack time
- **How to verify:**
  1. For each attack setting (0.1, 0.3, 1, 3, 10, 30 ms):
     - Apply step input from silence to 0dB
     - Measure time to reach 63.2% of final value
     - Verify within 10% of specified time
- **Priority:** High

#### TEST-DSP-ENV-004: Release Time Accuracy
- **What it tests:** Release time constants are correctly implemented
- **Expected behavior:** Envelope should decay to 36.8% of initial within specified release time
- **How to verify:**
  1. For each release setting (100, 300, 600, 1200 ms):
     - Apply signal then remove abruptly
     - Measure time to reach 36.8% of initial
     - Verify within 10% of specified time
- **Priority:** High

#### TEST-DSP-ENV-005: Auto Release Program Dependency
- **What it tests:** Auto-release adapts based on gain reduction amount
- **Expected behavior:** Higher gain reduction should result in faster release
- **How to verify:**
  1. Enable auto-release mode
  2. Apply signal causing 6dB GR, measure effective release time
  3. Apply signal causing 12dB GR, measure effective release time
  4. Verify second case releases faster
- **Priority:** Medium

#### TEST-DSP-ENV-006: Stereo Link Behavior
- **What it tests:** Stereo linking blends between independent and linked detection
- **Expected behavior:** At 100% link, both channels should have same envelope (max of both)
- **How to verify:**
  1. Apply signal to left channel only
  2. At 100% link, verify right envelope equals left
  3. At 0% link, verify right envelope is zero
  4. At 50%, verify intermediate behavior
- **Priority:** High

---

### 1.2 BusCompressor Tests

#### TEST-DSP-COMP-001: Threshold Accuracy
- **What it tests:** Compression begins exactly at threshold
- **Expected behavior:** Signals below threshold pass unchanged; signals above are compressed
- **How to verify:**
  1. Set threshold to -20dB, ratio to 4:1
  2. Apply -21dB signal, verify output equals input
  3. Apply -19dB signal, verify gain reduction occurs
- **Priority:** Critical

#### TEST-DSP-COMP-002: Ratio Accuracy (Stepped)
- **What it tests:** Stepped ratio values (2:1, 4:1, 10:1) apply correctly
- **Expected behavior:** For 4:1 ratio, 4dB over threshold results in 1dB over output
- **How to verify:**
  1. For each ratio value:
     - Set threshold -20dB
     - Apply signal 8dB over threshold (-12dB input)
     - Measure output level
     - Verify output is threshold + (input - threshold) / ratio
- **Priority:** Critical

#### TEST-DSP-COMP-003: Ratio Accuracy (Continuous)
- **What it tests:** Continuous ratio control works across full range (1:1 to 20:1)
- **Expected behavior:** Compression follows standard ratio equation
- **How to verify:**
  1. Test at 1.5:1, 6:1, 15:1 ratios
  2. Apply signal 10dB over threshold
  3. Verify gain reduction matches expected values
- **Priority:** High

#### TEST-DSP-COMP-004: Soft Knee Operation
- **What it tests:** Knee parameter creates smooth transition around threshold
- **Expected behavior:** With 6dB knee, compression ramps from threshold-3dB to threshold+3dB
- **How to verify:**
  1. Set threshold -20dB, knee 6dB, ratio 4:1
  2. Sweep input from -30dB to -10dB
  3. Verify smooth transition (no abrupt changes in gain reduction)
  4. Verify at threshold-3dB: minimal compression
  5. Verify at threshold+3dB: full ratio applied
- **Priority:** Medium

#### TEST-DSP-COMP-005: Mix (Parallel Compression)
- **What it tests:** Dry/wet mix blends compressed and original signals
- **Expected behavior:** At 50% mix, output is average of dry and compressed
- **How to verify:**
  1. Apply heavy compression (10dB GR)
  2. At 100% mix, verify full compression
  3. At 0% mix, verify output equals input
  4. At 50% mix, verify output is average of dry and compressed
- **Priority:** High

#### TEST-DSP-COMP-006: Makeup Gain Accuracy
- **What it tests:** Makeup gain applies specified dB boost
- **Expected behavior:** Each dB of makeup gain increases output by 1dB
- **How to verify:**
  1. Apply steady -20dB signal (no compression)
  2. Add 6dB makeup gain
  3. Verify output is -14dB
- **Priority:** High

#### TEST-DSP-COMP-007: Auto Makeup Calculation
- **What it tests:** Auto makeup compensates for average gain reduction
- **Expected behavior:** Auto makeup should roughly compensate for compression at moderate settings
- **How to verify:**
  1. Enable auto makeup
  2. Apply signal causing known GR
  3. Verify output level is closer to input level than with auto makeup disabled
- **Priority:** Medium

#### TEST-DSP-COMP-008: Sidechain HPF Operation
- **What it tests:** Sidechain high-pass filter reduces bass pumping
- **Expected behavior:** Low frequencies should not trigger compression as strongly
- **How to verify:**
  1. Set SC HPF to 200Hz
  2. Apply 50Hz sine at -10dB: measure GR
  3. Apply 500Hz sine at -10dB: measure GR
  4. Verify 50Hz causes less GR than 500Hz
- **Priority:** Medium

#### TEST-DSP-COMP-009: Feed-Forward vs Feed-Back Topology
- **What it tests:** Different topologies have distinct sonic characteristics
- **Expected behavior:** Feedback topology should have slightly different transient response
- **How to verify:**
  1. Apply transient-rich material (drum hit)
  2. Compare waveforms between FF and FB modes
  3. Verify FB mode has softer transient handling (different attack shape)
- **Priority:** Medium

#### TEST-DSP-COMP-010: Gain Smoothing (Zipper Noise Prevention)
- **What it tests:** Rapid parameter changes don't cause clicks/pops
- **Expected behavior:** Gain changes should be smoothed to prevent audible artifacts
- **How to verify:**
  1. Play continuous audio
  2. Rapidly sweep threshold from 0 to -40dB
  3. Listen for clicks/pops (should be none)
  4. Analyze output for discontinuities
- **Priority:** High

---

### 1.3 Saturator Tests

#### TEST-DSP-SAT-001: Clean Mode Passthrough
- **What it tests:** Clean character mode passes signal unchanged
- **Expected behavior:** Output equals input bit-for-bit (no processing)
- **How to verify:**
  1. Set character to Clean
  2. Process known signal
  3. Compare output to input (should be identical)
- **Priority:** Critical

#### TEST-DSP-SAT-002: Warm Saturation Harmonics
- **What it tests:** Warm mode adds even harmonics
- **Expected behavior:** Sine wave should gain 2nd, 4th harmonic content
- **How to verify:**
  1. Set character to Warm with drive
  2. Process 1kHz sine wave
  3. FFT output and verify presence of 2kHz, 4kHz harmonics
- **Priority:** Medium

#### TEST-DSP-SAT-003: Punch Asymmetric Clipping
- **What it tests:** Punch mode applies asymmetric saturation
- **Expected behavior:** Positive and negative peaks should be processed differently
- **How to verify:**
  1. Set character to Punch
  2. Process sine wave
  3. Verify output waveform is asymmetric
  4. Verify odd harmonics present
- **Priority:** Medium

#### TEST-DSP-SAT-004: DC Blocker Operation
- **What it tests:** DC blocker removes DC offset introduced by saturation
- **Expected behavior:** Output should have no DC offset regardless of saturation
- **How to verify:**
  1. Apply asymmetric waveform through Punch mode
  2. Measure DC offset of output
  3. Verify DC offset is < 0.001
- **Priority:** High

#### TEST-DSP-SAT-005: Saturation Level Scaling
- **What it tests:** Drive parameter correctly scales saturation amount
- **Expected behavior:** Higher drive = more harmonic content
- **How to verify:**
  1. Process same input at drive 0.1, 0.3, 0.5
  2. Measure THD at each setting
  3. Verify THD increases with drive
- **Priority:** Medium

---

### 1.4 DSP Utility Tests

#### TEST-DSP-UTIL-001: dB to Linear Conversion
- **What it tests:** decibelsToLinear() accuracy
- **Expected behavior:** 0dB = 1.0, -6dB ~ 0.5, -20dB = 0.1
- **How to verify:** Unit test with known values
- **Priority:** Critical

#### TEST-DSP-UTIL-002: Linear to dB Conversion
- **What it tests:** linearToDecibels() accuracy and edge cases
- **Expected behavior:** 1.0 = 0dB, 0.5 ~ -6dB, 0.0 = -100dB (floor)
- **How to verify:** Unit test with known values, especially 0 input
- **Priority:** Critical

#### TEST-DSP-UTIL-003: Biquad Coefficient Calculation
- **What it tests:** HPF coefficient calculation produces valid filters
- **Expected behavior:** Filter should attenuate below cutoff frequency
- **How to verify:**
  1. Generate coefficients for 100Hz HPF
  2. Process 50Hz sine: verify significant attenuation
  3. Process 200Hz sine: verify minimal attenuation
- **Priority:** High

#### TEST-DSP-UTIL-004: Filter Edge Case Handling
- **What it tests:** Filter calculations handle edge cases gracefully
- **Expected behavior:** No NaN, Inf, or crashes with extreme parameters
- **How to verify:**
  1. Test with freq = 1Hz (very low)
  2. Test with freq = Nyquist-1 (very high)
  3. Test with Q = 0.1 (very low)
  4. Verify all produce valid coefficients
- **Priority:** High

---

## 2. UI Tests

### 2.1 Layout and Rendering Tests

#### TEST-UI-LAYOUT-001: All Controls Visible
- **What it tests:** No UI elements are cut off or hidden
- **Expected behavior:** All knobs, buttons, labels, meters fully visible within window bounds
- **How to verify:**
  1. Launch standalone plugin
  2. Take screenshot
  3. Verify all elements from list:
     - [ ] Title label "BUS GLUE"
     - [ ] Bypass button
     - [ ] Threshold knob + label
     - [ ] SC HPF knob + label
     - [ ] Ratio selector + label
     - [ ] Attack selector + label
     - [ ] Release selector + label
     - [ ] Knee knob + label
     - [ ] Stepped button
     - [ ] GR meter (horizontal)
     - [ ] Detection selector + label
     - [ ] Topology selector + label
     - [ ] VU meter (all numbers 0-20)
     - [ ] Character selector + label
     - [ ] Mix knob + label
     - [ ] Link knob + label
     - [ ] Auto button
     - [ ] Makeup knob + label
     - [ ] Input meter (stereo)
     - [ ] Output meter (stereo)
- **Priority:** Critical

#### TEST-UI-LAYOUT-002: Knob Values Not Overlapping
- **What it tests:** Knob value text boxes don't overlap with knob graphics
- **Expected behavior:** Text boxes are positioned below knobs with appropriate spacing
- **How to verify:**
  1. Set each knob to various values
  2. Verify text is fully readable
  3. Verify no overlap with knob graphics
- **Priority:** High

#### TEST-UI-LAYOUT-003: Consistent Section Spacing
- **What it tests:** Sections have logical, consistent spacing
- **Expected behavior:** Equal padding between sections, no cramped areas
- **How to verify:**
  1. Measure pixel distances between section dividers
  2. Verify spacing matches design intent
  3. No random empty gaps
- **Priority:** Medium

#### TEST-UI-LAYOUT-004: VU Meter Needle Range
- **What it tests:** VU meter needle covers full 0-20dB range
- **Expected behavior:** Needle moves from 0 position to 20 position as GR increases
- **How to verify:**
  1. Apply no compression: needle at 0
  2. Apply heavy compression: needle moves toward 20
  3. Verify needle doesn't clip/cut off at extremes
- **Priority:** High

#### TEST-UI-LAYOUT-005: Background Image Rendering
- **What it tests:** Background image loads and displays correctly
- **Expected behavior:** Image visible behind controls with dark overlay
- **How to verify:**
  1. Verify image file exists and loads
  2. Verify 70% dark overlay applied
  3. Verify controls remain readable against background
- **Priority:** Low

---

### 2.2 Control Responsiveness Tests

#### TEST-UI-CTRL-001: Knob Dragging Response
- **What it tests:** Knobs respond smoothly to mouse drag
- **Expected behavior:** Continuous value changes while dragging, no jitter
- **How to verify:**
  1. Click and drag each knob
  2. Verify smooth value changes
  3. Verify value display updates in real-time
- **Priority:** High

#### TEST-UI-CTRL-002: ComboBox Selection
- **What it tests:** Dropdown menus work correctly
- **Expected behavior:** Menu opens on click, selection updates parameter
- **How to verify:**
  1. Click each combo box
  2. Verify menu appears
  3. Select each option
  4. Verify parameter updates
- **Priority:** High

#### TEST-UI-CTRL-003: Toggle Button States
- **What it tests:** Toggle buttons (Bypass, Auto, Stepped) work correctly
- **Expected behavior:** Visual state matches parameter state, toggles on click
- **How to verify:**
  1. Click each toggle button
  2. Verify LED indicator changes
  3. Verify parameter value changes
  4. Verify audio behavior changes accordingly
- **Priority:** High

#### TEST-UI-CTRL-004: Stepped Ratio Toggle
- **What it tests:** Stepped ratio button switches between combo box and continuous knob
- **Expected behavior:** When Stepped is on, show combo; when off, show knob
- **How to verify:**
  1. With Stepped on: verify ratio combo box visible, continuous slider hidden
  2. With Stepped off: verify continuous slider visible, combo box hidden
  3. Verify correct ratio applies to compression in each mode
- **Priority:** High

---

### 2.3 Meter Update Tests

#### TEST-UI-METER-001: Input Meter Accuracy
- **What it tests:** Input meters reflect actual input levels
- **Expected behavior:** Meter segments light up proportionally to input level
- **How to verify:**
  1. Apply -6dB signal: verify meter shows ~75% height
  2. Apply -20dB signal: verify meter shows ~30% height
  3. Apply 0dB signal: verify meter near full
- **Priority:** High

#### TEST-UI-METER-002: Output Meter Accuracy
- **What it tests:** Output meters reflect actual output levels
- **Expected behavior:** Meter includes effect of compression and makeup gain
- **How to verify:**
  1. Apply signal with makeup gain: verify output meter higher than input
  2. Apply heavy compression with no makeup: verify output meter lower
- **Priority:** High

#### TEST-UI-METER-003: GR Meter Response
- **What it tests:** Horizontal GR meter shows current gain reduction
- **Expected behavior:** LEDs light from right to left as GR increases (0 = right, 20 = left)
- **How to verify:**
  1. No compression: only rightmost LED lit
  2. 10dB GR: middle LEDs lit
  3. 20dB GR: all LEDs lit
- **Priority:** High

#### TEST-UI-METER-004: VU Meter Animation
- **What it tests:** VU meter needle has ballistic response
- **Expected behavior:** Needle smoothly animates, not instant jumps
- **How to verify:**
  1. Apply sudden compression
  2. Observe needle movement
  3. Verify smooth animation (not instant)
  4. Verify needle settles at correct position
- **Priority:** Medium

#### TEST-UI-METER-005: Meter Update Rate
- **What it tests:** Meters update at consistent rate
- **Expected behavior:** 30Hz refresh rate (per timerCallback)
- **How to verify:**
  1. Verify timer is running at 30Hz
  2. Verify no visible stuttering in meter animation
- **Priority:** Medium

---

## 3. Integration Tests

### 3.1 AU Validation Tests

#### TEST-INT-AU-001: auval Validation Pass
- **What it tests:** Plugin passes Apple's AU validation tool
- **Expected behavior:** `auval -v aufx BuG1 Flet` returns no errors
- **How to verify:**
  1. Build AU plugin
  2. Run `auval -v aufx BuG1 Flet`
  3. Verify all tests pass
  4. Verify "PASS" result
- **Priority:** Critical

#### TEST-INT-AU-002: Bus Layout Negotiation
- **What it tests:** Plugin correctly reports supported bus layouts
- **Expected behavior:** Accept stereo in/out, reject other configurations
- **How to verify:**
  1. Verify isBusesLayoutSupported accepts stereo/stereo
  2. Verify it rejects mono/stereo, stereo/mono
  3. Verify it accepts disabled/disabled (for AU compatibility)
- **Priority:** Critical

#### TEST-INT-AU-003: State Save/Restore
- **What it tests:** Plugin correctly saves and restores all parameters
- **Expected behavior:** After save/load, all parameters match original values
- **How to verify:**
  1. Set all parameters to non-default values
  2. Call getStateInformation()
  3. Reset to defaults
  4. Call setStateInformation()
  5. Verify all parameters restored
- **Priority:** Critical

#### TEST-INT-AU-004: State Robustness
- **What it tests:** Plugin handles malformed state data gracefully
- **Expected behavior:** Invalid state data should not crash, just be ignored
- **How to verify:**
  1. Call setStateInformation with null/empty data
  2. Call with truncated data
  3. Call with random bytes
  4. Verify no crash in any case
- **Priority:** High

---

### 3.2 Logic Pro Compatibility Tests

#### TEST-INT-LOGIC-001: Plugin Loading (No Hang)
- **What it tests:** Plugin loads in Logic Pro without hanging
- **Expected behavior:** Plugin UI appears within 5 seconds of selection
- **How to verify:**
  1. Open Logic Pro
  2. Create new project with audio track
  3. Add Bus Glue as audio effect
  4. Verify plugin loads without spinning wheel
  5. Verify UI is responsive
- **Priority:** Critical

#### TEST-INT-LOGIC-002: Playback Processing
- **What it tests:** Audio processes correctly during Logic playback
- **Expected behavior:** Audio passes through with compression applied
- **How to verify:**
  1. Load plugin in Logic
  2. Play audio through it
  3. Verify audio is processed (meters move, compression audible)
  4. Verify no dropouts or glitches
- **Priority:** Critical

#### TEST-INT-LOGIC-003: Parameter Automation
- **What it tests:** All parameters can be automated in Logic
- **Expected behavior:** Automation lanes work for all exposed parameters
- **How to verify:**
  1. Create automation lane for each parameter
  2. Draw automation curves
  3. Verify parameters follow automation during playback
- **Priority:** High

#### TEST-INT-LOGIC-004: Project Save/Load
- **What it tests:** Plugin state persists across Logic project save/load
- **Expected behavior:** All settings restored when project reopened
- **How to verify:**
  1. Configure plugin with non-default settings
  2. Save Logic project
  3. Close and reopen project
  4. Verify all plugin settings restored
- **Priority:** High

#### TEST-INT-LOGIC-005: Multiple Instances
- **What it tests:** Multiple plugin instances work independently
- **Expected behavior:** Each instance has independent state and processing
- **How to verify:**
  1. Insert plugin on two different tracks
  2. Set different parameters on each
  3. Verify each processes independently
  4. Verify no cross-talk or shared state issues
- **Priority:** Medium

---

### 3.3 Standalone Application Tests

#### TEST-INT-STANDALONE-001: App Launch
- **What it tests:** Standalone app launches successfully
- **Expected behavior:** App window appears with full UI
- **How to verify:**
  1. Build standalone target
  2. Launch .app
  3. Verify window appears
  4. Verify all UI elements present
- **Priority:** High

#### TEST-INT-STANDALONE-002: Audio Device Selection
- **What it tests:** Standalone correctly interfaces with audio devices
- **Expected behavior:** Can select input/output devices, audio passes through
- **How to verify:**
  1. Launch standalone
  2. Open audio settings
  3. Select audio interface
  4. Play audio through it
  5. Verify processing works
- **Priority:** Medium

---

## 4. Regression Tests

These tests are specifically designed to catch the issues documented in ISSUE.md and git history.

### 4.1 Critical: AU Loading Hang Prevention

#### TEST-REG-001: No Timer Start in Constructor
- **What it tests:** Timer is not started synchronously in editor constructor
- **Expected behavior:** Timer start is deferred via MessageManager::callAsync
- **How to verify:**
  1. Review PluginEditor constructor code
  2. Verify startTimerHz is called inside callAsync lambda
  3. Verify isShowing() check before timer start
- **Priority:** Critical
- **Issue Reference:** ISSUE.md - "Plugin hangs when loading in Logic Pro"

#### TEST-REG-002: No Blocking Operations in Constructor
- **What it tests:** No operations in constructor that could block message thread
- **Expected behavior:** Constructor completes quickly without waiting for external resources
- **How to verify:**
  1. Profile constructor execution time
  2. Verify < 100ms total constructor time
  3. Verify no mutex locks, file I/O waits, or network calls
- **Priority:** Critical
- **Issue Reference:** ISSUE.md - Deadlock investigation

#### TEST-REG-003: Sidechain Bus Removed
- **What it tests:** Plugin only declares main stereo I/O, no sidechain bus
- **Expected behavior:** BusesProperties only has single input and output bus
- **How to verify:**
  1. Verify constructor uses:
     ```cpp
     .withInput("Input", stereo, true)
     .withOutput("Output", stereo, true)
     ```
  2. Verify no `.withInput("Sidechain", ...)` line
  3. Verify getTotalNumInputChannels() returns 2
- **Priority:** Critical
- **Issue Reference:** ISSUE.md - "Sidechain bus configuration is the primary suspect"

---

### 4.2 High: Correct Plugin Factory Function

#### TEST-REG-004: createPluginFilter Exists
- **What it tests:** Plugin exports correct factory function
- **Expected behavior:** createPluginFilter() function exists and returns processor
- **How to verify:**
  1. Verify function signature: `juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()`
  2. Verify it returns `new BusGlueAudioProcessor()`
  3. Verify NOT named createPluginInstance (incorrect)
- **Priority:** Critical
- **Issue Reference:** ISSUE.md - "Initial commit had createPluginInstance()"

---

### 4.3 High: Plugin Characteristics

#### TEST-REG-005: No MIDI Plugin Flags
- **What it tests:** Plugin is correctly identified as audio effect, not MIDI
- **Expected behavior:** Plugin doesn't claim to be synth or MIDI effect
- **How to verify:**
  1. Verify acceptsMidi() returns false
  2. Verify producesMidi() returns false
  3. Verify isMidiEffect() returns false
  4. Verify jucer has empty pluginCharacteristicsValue
- **Priority:** High
- **Issue Reference:** ISSUE.md - "Incorrect Plugin Characteristics"

#### TEST-REG-006: Correct Manufacturer Code
- **What it tests:** Manufacturer code matches registered plugins
- **Expected behavior:** Manufacturer code is "Flet" (not "IFAu")
- **How to verify:**
  1. Check jucer file for pluginManufacturerCode="Flet"
  2. Verify plugin appears under "Fletcher" in Logic
- **Priority:** High
- **Issue Reference:** ISSUE.md - "Manufacturer Code Mismatch"

---

### 4.4 Medium: Bus Layout Support

#### TEST-REG-007: Disabled Layout Acceptance
- **What it tests:** Plugin accepts disabled bus layout for AU compatibility
- **Expected behavior:** isBusesLayoutSupported returns true for disabled in/out
- **How to verify:**
  1. Create BusesLayout with disabled input and output
  2. Verify isBusesLayoutSupported returns true
  3. This is required for AU host queries during validation
- **Priority:** High
- **Issue Reference:** ISSUE.md - "AU hosts query this"

#### TEST-REG-008: Stereo-Only Enforcement
- **What it tests:** Plugin rejects non-stereo layouts
- **Expected behavior:** Only stereo/stereo accepted (or disabled/disabled)
- **How to verify:**
  1. Test mono input: should return false
  2. Test mono output: should return false
  3. Test stereo input with mono output: should return false
  4. Test stereo/stereo: should return true
- **Priority:** High

---

### 4.5 Medium: State Management

#### TEST-REG-009: Exception Handling in setStateInformation
- **What it tests:** State restoration handles exceptions gracefully
- **Expected behavior:** Malformed data causes no crash, just logged warning
- **How to verify:**
  1. Call setStateInformation with corrupt data
  2. Verify no exception propagates
  3. Verify plugin continues to function
- **Priority:** High
- **Issue Reference:** Code shows try-catch blocks added for robustness

---

### 4.6 UI Regression Tests

#### TEST-REG-010: VU Meter Numbers Visible
- **What it tests:** All VU meter scale numbers (0, 3, 6, 10, 15, 20) are visible
- **Expected behavior:** Numbers not cut off by meter bounds
- **How to verify:**
  1. Take screenshot of VU meter
  2. Verify all 6 scale labels visible
  3. Verify "10" specifically visible (was cut off in earlier version)
- **Priority:** High
- **Issue Reference:** Git history - "VU meter: frame bottom now bisects pivot dot, '10' label visible"

#### TEST-REG-011: VU Meter Needle Not Cut Off
- **What it tests:** VU meter needle fully visible at all positions
- **Expected behavior:** Needle visible from 0dB to 20dB positions
- **How to verify:**
  1. Apply varying amounts of compression
  2. Verify needle visible at all positions
  3. Verify needle doesn't extend beyond meter bounds
- **Priority:** High
- **Issue Reference:** Git history - Multiple VU meter sizing commits

#### TEST-REG-012: GR Meter Positioning
- **What it tests:** GR meter properly positioned in dynamics section
- **Expected behavior:** GR meter between Knee knob and Detection selector
- **How to verify:**
  1. Verify GR meter horizontal orientation
  2. Verify positioned equidistant between Knee bottom and Detection top
  3. Verify full width of dynamics section (minus padding)
- **Priority:** Medium
- **Issue Reference:** Git history - "Moved GR meter to dynamics section"

#### TEST-REG-013: Makeup Knob Bottom Alignment
- **What it tests:** Makeup knob bottom-aligned with other bottom elements
- **Expected behavior:** Bottom of Makeup knob aligns with bottom of other knobs
- **How to verify:**
  1. Verify Makeup knob bottom edge at same Y as Mix/Link knobs
  2. Verify label above knob, not overlapping
- **Priority:** Medium
- **Issue Reference:** Git history - "Moved Makeup knob to bottom, aligned with other bottom elements"

#### TEST-REG-014: Background Image Crop
- **What it tests:** Background image shows intended portion (50/50 trees/buildings)
- **Expected behavior:** Top portion shows trees, bottom shows buildings
- **How to verify:**
  1. Verify srcY calculation uses 0.25 factor (shifts up for more trees)
  2. Visual inspection confirms balanced composition
- **Priority:** Low
- **Issue Reference:** Git history - "Shifted background image up for 50/50 trees/buildings split"

---

## Test Execution Checklist

### Before Each Release:

1. **All Critical Tests Must Pass**
   - [ ] TEST-DSP-ENV-001, 002 (detection modes)
   - [ ] TEST-DSP-COMP-001, 002 (threshold, ratio)
   - [ ] TEST-DSP-SAT-001 (clean passthrough)
   - [ ] TEST-DSP-UTIL-001, 002 (dB conversions)
   - [ ] TEST-UI-LAYOUT-001 (all controls visible)
   - [ ] TEST-INT-AU-001 (auval pass)
   - [ ] TEST-INT-LOGIC-001 (no hang on load)
   - [ ] TEST-REG-001, 002, 003, 004 (constructor, factory, buses)

2. **All High Priority Tests Must Pass**
   - [ ] All envelope follower timing tests
   - [ ] Compression parameter tests
   - [ ] UI control responsiveness tests
   - [ ] Meter accuracy tests
   - [ ] Logic Pro integration tests

3. **Medium Priority Tests Should Pass**
   - Document any known issues for medium priority failures

4. **Low Priority Tests Are Informational**
   - Log failures but don't block release

---

## Automated Testing Notes

### Recommended Test Framework
- Use JUCE's built-in unit test framework for DSP tests
- Use screenshot comparison tools for UI layout tests
- Create shell scripts for AU validation and Logic Pro testing

### CI/CD Integration
```bash
# Build and test script example
xcodebuild -project Builds/MacOSX/BusGlue.xcodeproj \
           -scheme "BusGlue - AU" \
           -configuration Release build

# AU Validation
auval -v aufx BuG1 Flet

# Unit tests (if implemented)
./Builds/MacOSX/build/Release/BusGlueTests
```

### Test Data
- Store reference audio files for DSP testing
- Store reference screenshots for UI comparison
- Version control test fixtures

---

*Document Version: 1.0*
*Created: 2026-01-31*
*Based on: ISSUE.md, git history, source code analysis*
