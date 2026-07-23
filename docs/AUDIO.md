# Audio pipeline

## Runtime

- Nintendo 3DS NDSP is preferred; CSND is the runtime fallback when NDSP initialization fails.
- Channel `0` is reserved for the diagnostic stereo tone.
- Channels `1`–`6` form a priority-partitioned SFX pool.
- The mission-music loop uses its own channel after the SFX pool.
- Every new cue resets only the selected SFX channel; rendering and simulation never wait for audio.
- Failure of both audio backends or sample allocation disables audio without failing game startup.

## Current cues

All samples are generated procedurally at startup and authored for this project:

- successful tower build;
- rejected tower build;
- wave start;
- crossbow shot;
- real projectile impact;
- enemy death;
- base damage;
- victory;
- defeat.

No external WAV, SDK or copyrighted game asset is included.

## Format and budget

- mono signed PCM16 for gameplay cues and music;
- stereo signed PCM16 for the diagnostic tone;
- 22,050 Hz;
- short SFX envelopes with no allocation during a gameplay frame;
- six simultaneous SFX channels split into combat, action and protected result pools;
- immutable sample buffers allocated in linear memory and flushed once after generation.

## Exact event contract

`AudioEventRouter` is platform-independent and unit-tested. Runtime input is built from monotonic counters owned by the systems that resolve each event:

- `ProjectilePool::shotEventCount()` increments only after a projectile was successfully allocated;
- `ProjectilePool::impactEventCount()` increments only after `ProjectileUpdateResult::Impact`; cancellation after a lost target stays silent;
- `Wave::deathEventCount()` increments when a dead enemy is resolved exactly once;
- `Wave::baseDamageEventCount()` increments when an enemy reaches the base and is resolved exactly once.

The router compares counters between frames. Shot and impact cues retain short cooldowns to limit dense combat series. Enemy-death and base-damage events are never discarded by cooldown. Build confirmation is routed directly from the tested `BuildAttemptResult` contract.

Reset stops all active wave buffers and snapshots the current counters before gameplay. Shutdown clears channels, frees linear sample memory and exits only the backend that initialized successfully.
