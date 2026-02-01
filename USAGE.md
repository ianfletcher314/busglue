# Bus Glue - Usage Guide

**SSL/API Style Bus Compressor**

Bus Glue is a classic bus compressor designed to add punch, cohesion, and "glue" to your mixes. Inspired by the legendary SSL G-Series and API 2500 compressors, it features stepped attack/release times, multiple character modes, and flexible sidechain options.

---

## Use Cases in Modern Rock Production

### Drum Bus Processing

The drum bus is where Bus Glue truly shines. It can transform a collection of individual drum tracks into a cohesive, punchy kit.

**Punchy Modern Rock Drums:**
- Threshold: -15 to -20 dB
- Ratio: 4:1
- Attack: 10 ms (lets transients through)
- Release: 300 ms or Auto
- Mix: 70-100%
- Character: Punch or Aggressive
- SC HPF: 80-100 Hz (prevents kick from pumping)

**Aggressive Metal/Hardcore Drums:**
- Threshold: -20 to -25 dB
- Ratio: 10:1
- Attack: 0.3 ms (smash those transients)
- Release: 100 ms
- Mix: 100%
- Character: Aggressive
- Topology: Feed-Back

**Parallel "New York" Drum Compression:**
- Threshold: -30 dB (heavy compression)
- Ratio: 10:1
- Attack: 0.1 ms
- Release: Auto
- Mix: 30-50% (blend to taste)
- Character: Punch

### Guitar Bus / Individual Tracks

Guitar buses benefit from subtle glue compression to unify multiple mic'd amps or layered parts.

**Clean Guitar Glue:**
- Threshold: -12 to -15 dB
- Ratio: 2:1
- Attack: 10-30 ms
- Release: 600 ms or Auto
- Mix: 100%
- Character: Clean or Warm
- Knee: 6 dB (soft knee)

**Heavy Rhythm Guitars:**
- Threshold: -18 dB
- Ratio: 4:1
- Attack: 3-10 ms
- Release: 300-600 ms
- Character: Aggressive
- SC HPF: 100-150 Hz

### Bass Guitar

Bass guitar needs careful compression to maintain punch while controlling dynamics.

**Rock Bass - Punchy and Present:**
- Threshold: -15 to -20 dB
- Ratio: 4:1
- Attack: 10 ms
- Release: 300 ms
- Character: Punch
- SC HPF: 80 Hz
- Mix: 80-100%

**Fingerstyle Bass - Smooth and Even:**
- Threshold: -12 dB
- Ratio: 2:1
- Attack: 30 ms
- Release: 600 ms or Auto
- Character: Warm
- Knee: 8-12 dB

### Vocals

While VoxProc may be more purpose-built for vocals, Bus Glue can add character and glue to vocal buses or backing vocal stacks.

**Lead Vocal - Subtle Character:**
- Threshold: -10 to -15 dB
- Ratio: 2:1
- Attack: 10 ms
- Release: Auto
- Character: Warm
- Knee: 6 dB
- Mix: 70-100%

**Backing Vocal Stack:**
- Threshold: -18 dB
- Ratio: 4:1
- Attack: 3 ms
- Release: 300 ms
- Mix: 100%
- Character: Clean

### Mix Bus / Mastering

The mix bus is the classic application for SSL-style compression.

**Mix Bus Glue - Subtle:**
- Threshold: -15 to -18 dB (1-3 dB gain reduction)
- Ratio: 2:1
- Attack: 10-30 ms
- Release: Auto
- Mix: 100%
- Character: Clean
- SC HPF: 60-80 Hz

**Mix Bus - More Aggressive:**
- Threshold: -20 dB (3-5 dB gain reduction)
- Ratio: 4:1
- Attack: 3-10 ms
- Release: 300 ms
- Character: Punch
- SC HPF: 100 Hz

---

## Recommended Settings

### Quick Reference Table

| Application | Threshold | Ratio | Attack | Release | Character | SC HPF |
|------------|-----------|-------|--------|---------|-----------|--------|
| Drum Bus (punchy) | -15 to -20 dB | 4:1 | 10 ms | 300 ms/Auto | Punch | 80-100 Hz |
| Drum Bus (smash) | -25 dB | 10:1 | 0.1-0.3 ms | 100 ms | Aggressive | 100 Hz |
| Guitar Bus | -15 dB | 2:1 | 10-30 ms | 600 ms | Clean/Warm | Off |
| Bass | -18 dB | 4:1 | 10 ms | 300 ms | Punch | 80 Hz |
| Mix Bus | -15 to -18 dB | 2:1 | 10-30 ms | Auto | Clean | 60-80 Hz |
| Parallel Crush | -30 dB | 10:1 | 0.1 ms | Auto | Aggressive | Off |

### Character Mode Guide

- **Clean**: Transparent compression, minimal coloration - best for mix bus and acoustic instruments
- **Punch**: Emphasizes transients and adds body - ideal for drums and bass
- **Warm**: Adds subtle harmonic saturation - great for guitars and vocals
- **Aggressive**: Maximum punch and saturation - perfect for heavy rock/metal drums

### Topology Options

- **Feed-Forward**: More precise, predictable compression - modern SSL sound
- **Feed-Back**: More musical, program-dependent response - vintage API/Neve vibe

---

## Signal Flow Tips

### Where to Place Bus Glue

1. **After EQ, Before Effects**: Place on your drum/guitar bus after any subtractive EQ but before time-based effects (reverb, delay)

2. **First in Mix Bus Chain**: For mix bus use, place Bus Glue early in your chain, typically before any limiting or final EQ

3. **Parallel Processing**: Use the Mix knob for parallel compression, or set up a parallel aux for more control

### Gain Staging

- Aim for proper gain staging into Bus Glue - peaks around -6 to -3 dBFS
- Use Makeup Gain to compensate for gain reduction
- Enable Auto Makeup for quick A/B comparisons

---

## Combining with Other Plugins

### Drum Bus Chain
1. **EQ** (subtractive) - remove problem frequencies
2. **Bus Glue** - add punch and glue
3. **TapeWarm** (optional) - add analog warmth
4. **StereoImager** - widen overheads/rooms

### Guitar Bus Chain
1. **Bus Glue** (subtle) - unify multiple takes
2. **TapeWarm** - add analog character
3. **MasterBus EQ** - final shaping

### Mix Bus Chain
1. **Bus Glue** - subtle glue compression (2-3 dB GR)
2. **MasterBus** - mastering EQ and additional compression
3. **Automaster** - final limiting and loudness

---

## Quick Start Guide

**Get a great drum bus sound in 30 seconds:**

1. Insert Bus Glue on your drum bus
2. Set **Ratio** to 4:1
3. Set **Attack** to 10 ms
4. Set **Release** to Auto
5. Set **Character** to Punch
6. Enable **SC HPF** at 80 Hz
7. Lower **Threshold** until you see 3-6 dB of gain reduction on peaks
8. Adjust **Makeup Gain** to match bypass level
9. Fine-tune **Mix** to taste (start at 100%, reduce if too aggressive)

**Get solid mix bus glue in 30 seconds:**

1. Insert Bus Glue on your mix bus
2. Set **Ratio** to 2:1
3. Set **Attack** to 30 ms
4. Set **Release** to Auto
5. Set **Character** to Clean
6. Enable **SC HPF** at 60 Hz
7. Set **Threshold** for 1-2 dB of gain reduction
8. Enable **Auto Makeup**
9. A/B with bypass to confirm improvement
