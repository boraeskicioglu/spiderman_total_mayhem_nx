# Ultimate Spider-Man: Total Mayhem NX v1.0.0 — Initial Release

The first public release of the Nintendo Switch port of **Ultimate Spider-Man: Total Mayhem HD** for Android 1.0.8.

## Highlights

- Runs the original 32-bit ARM Android game library directly in AArch32 mode.
- 1280×720 widescreen output.
- Gameplay targeting 60 FPS on real Switch hardware.
- Joy-Con / Pro Controller gameplay controls.
- Gamepad-driven menu cursor with A/B navigation.
- Native touchscreen support.
- Mobile HUD hidden while using a controller while preserving context/NPC/QTE prompts.
- Working save files and persistent settings.
- Native OGG/Vorbis music, SFX and voice playback through Switch `audout`.
- S3TC/DXT compatibility fallback for correct textures.
- Switch-specific load/performance handling.
- HOME-menu forwarder/self-update flow through the `android32` launcher.

## Requirements

This release does **not** include the game.

You need your own legally obtained Android **1.0.8** copy (`com.gameloft.android.GAND.GloftSMHP.ML`, versionCode 108) and the original `/sdcard/gameloft/games/GloftSMHP/` game-data directory.

See `README.md` for the complete SD-card layout and setup instructions.

## Release File

`spiderman_total_mayhem_nx.nro`

Place it in `sd:/switch/spiderman_total_mayhem_nx/`, install its forwarder with sphaira and launch the resulting HOME-menu icon.

## Notes

Only Android 1.0.8 / versionCode 108 has been tested.

Please include `debug.log` and `crash.log` when reporting a problem. Do not upload copyrighted APK or game-data files to GitHub issues.
