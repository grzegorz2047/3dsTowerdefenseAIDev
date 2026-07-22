# Audio pipeline

## Runtime

- Nintendo 3DS NDSP through libctru.
- Channel `0` is reserved for future music and ambient.
- Channels `1`–`6` form a round-robin SFX pool.
- Every new cue resets only the selected SFX channel; rendering and simulation never wait for audio.
- Failure of `ndspInit()` or sample allocation disables audio without failing game startup.

## Current cues

All samples are generated procedurally at startup and authored for this project:

- successful tower build;
- rejected tower build;
- wave start;
- crossbow shot;
- projectile hit/resolution;
- victory;
- defeat.

No external WAV, SDK or copyrighted game asset is included.

## Format and budget

- mono signed PCM16;
- 22,050 Hz;
- 70–520 ms per cue;
- approximately 65 KiB total generated sample memory;
- six simultaneous SFX channels;
- immutable sample buffers allocated in linear memory and flushed once after generation;
- no allocation during a gameplay frame.

## Event contract

`AudioEventRouter` is platform-independent and unit-tested. It emits one-shot cue masks from changes in tutorial phase and active projectile count. Build confirmation is routed directly from the tested `BuildAttemptResult` contract.

Reset stops all active wave buffers and reinitializes the router. Shutdown clears channels, frees linear sample memory and calls `ndspExit()` only when initialization succeeded.
