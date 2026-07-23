# Audio pipeline

## Runtime

- Nintendo 3DS NDSP is preferred; CSND is the runtime fallback when NDSP initialization fails.
- Channel `0` is reserved for the diagnostic stereo tone.
- Channels `1`–`6` form a priority-partitioned SFX pool.
- Channel `7` carries the preparation loop, channel `8` the combat loop and channel `9` the ambient layer.
- Music layers start once and remain looped; phase transitions change channel gain over 450 ms instead of restarting buffers.
- Every new cue resets only the selected SFX channel; rendering and simulation never wait for audio.
- Failure of both audio backends or sample allocation disables audio without failing game startup.

## Procedural music and ambient

All audio is generated deterministically at startup and authored for this project:

- calm 16-beat preparation loop for building and ready-to-start phases;
- faster 16-beat combat loop with a light procedural pulse;
- low-volume wind and distant-bird ambient layer;
- victory and defeat remain short protected SFX so mission results stay readable above the music.

Every loop begins and ends at zero amplitude. During a phase change the previous layer fades down while the target layer fades up. Ambient follows the same gain ramp. The result phases fade all music to silence so the victory or defeat cue is not masked.

`GameSettings::musicEnabled` is persisted independently from `soundEnabled`. On the campaign screen `X` toggles SFX and `B` toggles music. Disabling music leaves the SFX router and channels untouched.

No external WAV, SDK or copyrighted game asset is included.

## Current SFX cues

- successful tower build;
- rejected tower build;
- wave start;
- crossbow shot;
- real projectile impact;
- enemy death;
- base damage;
- victory;
- defeat.

## Format and budget

- mono signed PCM16 for gameplay cues, music and ambient;
- stereo signed PCM16 for the diagnostic tone;
- 22,050 Hz;
- short SFX envelopes with no allocation during a gameplay frame;
- six simultaneous SFX channels split into combat, action and protected result pools;
- three immutable looping music buffers allocated once in linear memory;
- approximately 760 KiB for the three procedural layers at the current durations;
- no mixing allocation or sample generation during gameplay.

## Exact event contract

`AudioEventRouter` is platform-independent and unit-tested. Runtime input is built from monotonic counters owned by the systems that resolve each event:

- `ProjectilePool::shotEventCount()` increments only after a projectile was successfully allocated;
- `ProjectilePool::impactEventCount()` increments only after `ProjectileUpdateResult::Impact`; cancellation after a lost target stays silent;
- `Wave::deathEventCount()` increments when a dead enemy is resolved exactly once;
- `Wave::baseDamageEventCount()` increments when an enemy reaches the base and is resolved exactly once.

The router compares counters between frames. Shot and impact cues retain short cooldowns to limit dense combat series. Enemy-death and base-damage events are never discarded by cooldown. Build confirmation is routed directly from the tested `BuildAttemptResult` contract.

Shutdown stops all looping and one-shot channels, frees linear sample memory and exits only the backend that initialized successfully.
