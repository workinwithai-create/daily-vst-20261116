# Today’s Instrument: Velvet Afterglow

A warm, living string-like pad synthesizer that responds to aftertouch for expressive vibrato and uses the sustain pedal for natural holds.

## Build Notes

### Prerequisites
- CMake 3.22+
- C++20 compatible compiler (Clang 14+, GCC 11+, MSVC 2022)
- Git

### Local Build
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

Outputs appear in `build/VelvetAfterglow_artefacts/Release/`:
- VST3: `VST3/Velvet Afterglow.vst3`
- Standalone: `Standalone/Velvet Afterglow`

### GitHub Actions
The included workflow builds Release VST3 + Standalone on macOS and Windows, packages them as zips, and attaches them to a GitHub Release on push to `main` or manual dispatch.

## Features
- 16-voice polyphonic subtractive pad
- Detuned dual oscillators
- Simple lowpass filter modulated by aftertouch
- ADSR envelope
- Aftertouch-controlled vibrato depth
- Sustain pedal support (MIDI CC 64)
- Safety limiter
- Full state save/load via APVTS
- Dark cinematic UI

Company: MPJRecords  
Manufacturer Code: MPJR  
Plugin Code: VafG
