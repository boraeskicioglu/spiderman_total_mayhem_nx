<div align="center">

<img src="launcher/icon.jpg" alt="Ultimate Spider-Man: Total Mayhem HD" width="180">

# spiderman_total_mayhem_nx

**Ultimate Spider-Man: Total Mayhem HD on Nintendo Switch**

An unofficial Nintendo Switch wrapper/port of the Android version of **Ultimate Spider-Man: Total Mayhem HD** by Gameloft.

[![Switch](https://img.shields.io/badge/Nintendo_Switch-Homebrew-E60012?style=for-the-badge&logo=nintendoswitch&logoColor=white)](#)
[![Version](https://img.shields.io/badge/version-v1.0.0-4C8BF5?style=for-the-badge)](#)
[![AArch32](https://img.shields.io/badge/AArch32-32--bit-0091BD?style=for-the-badge&logo=arm&logoColor=white)](#)
[![OpenGL ES](https://img.shields.io/badge/OpenGL_ES-1.1-5586A4?style=for-the-badge&logo=opengl&logoColor=white)](#)

</div>

---

## About

`spiderman_total_mayhem_nx` runs the original 32-bit Android build of **Ultimate Spider-Man: Total Mayhem HD** on Nintendo Switch.

The port targets Android **1.0.8**:

```text
Package:     com.gameloft.android.GAND.GloftSMHP.ML
Version:     1.0.8
VersionCode: 108
ABI:         armeabi / AArch32
```

The original `libspiderman.so` is loaded inside a 32-bit Horizon OS process using the open-source [`android32`](https://github.com/aks796/android32) runtime. The wrapper provides the Android/Bionic/JNI interfaces, OpenGL ES, filesystem, input, audio and platform services expected by the original game.

This is **not an emulator** and the game is not recompiled from source. The original ARM Android game library runs directly in AArch32 mode on Switch.

No APK, `libspiderman.so`, levels, music, sound effects, textures or other runtime game data are included. You must provide files from your own legally obtained copy of the game.

---

## Features

- Original Android **1.0.8** game running directly in AArch32 on Nintendo Switch.
- **1280×720** widescreen output.
- Gameplay targeting **60 FPS**.
- Joy-Con and Pro Controller support.
- Controller navigation in the original touch-oriented menus.
- Native touchscreen support.
- Permanent Android joystick/action HUD hidden while using a controller.
- Context, NPC and QTE prompts remain visible.
- Persistent save data and settings.
- Working music, sound effects and voice audio through a native OGG/Vorbis mixer.
- DXT/S3TC texture fallback for correct rendering on Switch.
- Switch-specific performance handling and load-time CPU boost behavior.
- HOME-menu forwarder flow through the `android32` launcher.
- Debug/crash logging for troubleshooting.

---

## Controls

### Gameplay

| Nintendo Switch | Action |
| --- | --- |
| **Left Stick / D-Pad** | Move |
| **B** | Jump |
| **Y** | Punch / attack |
| **A** | Web / projectile action |
| **X** | Super action |
| **ZL** | Context / danger action |
| **+** | Pause |
| **Touchscreen** | Native touch input; temporarily shows the mobile controls |

### Menus

| Nintendo Switch | Action |
| --- | --- |
| **Left Stick / D-Pad** | Move the menu cursor |
| **A** | Select / tap |
| **B** | Back |
| **R3** | Recenter the menu cursor |
| **Touchscreen** | Native menu touch input |

---

## Installation

### Requirements

You need:

- A Nintendo Switch capable of running Atmosphère homebrew.
- [sphaira](https://github.com/ITotalJustice/sphaira) for the HOME-menu forwarder workflow.
- Your own legally obtained Android **1.0.8** copy of Ultimate Spider-Man: Total Mayhem HD.
- The original external Gameloft game-data folder.

Create:

```text
sd:/switch/spiderman_total_mayhem_nx/
```

Copy the release NRO into it:

```text
sd:/switch/spiderman_total_mayhem_nx/spiderman_total_mayhem_nx.nro
```

Place your Android 1.0.8 APK in the same directory. The tested build used:

```text
Spider-man-HD-1-0-8.apk
```

Copy the original game data so the layout contains:

```text
sd:/switch/spiderman_total_mayhem_nx/
├── spiderman_total_mayhem_nx.nro
├── Spider-man-HD-1-0-8.apk
└── gameloft/
    └── games/
        └── GloftSMHP/
            ├── configs.pack
            ├── entities.pack
            ├── editor.pack
            ├── comic2.pack
            ├── levelnew_01.pack
            ├── ...
            ├── levelnew_12.pack
            ├── logo.mp4
            └── sound/
                ├── MUSIC/
                ├── SFX/
                └── VFX/
```

The original Android path:

```text
/sdcard/gameloft/games/GloftSMHP/
```

is mapped to the folder above.

### First Launch

1. Open **sphaira**.
2. Find **Ultimate Spider-Man: Total Mayhem HD** in Homebrew.
3. Choose **Install Forwarder**.
4. Launch the new game icon from the Switch HOME menu.
5. The launcher installs the 32-bit game program and starts the wrapper.
6. On first setup the port extracts the required Android native library and builds its local runtime files.

After setup, files such as these are created in the same folder:

```text
libspiderman.so
classes.txt
debug.log
config.ini
gameloft/games/GloftSMHP/save.dat
gameloft/games/GloftSMHP/checkpoint.dat
gameloft/games/GloftSMHP/settings.dat
```

Keep the game folder when updating so your save data is preserved.

### Updating

Replace `spiderman_total_mayhem_nx.nro` with the newer release and launch the HOME-menu icon again. The launcher carries the new 32-bit payload and updates the installed game program.

---

## Building

### Requirements

- Docker
- Python 3
- [`android32`](https://github.com/aks796/android32) in `runtime/`
- [`libnx32`](https://github.com/aks796/libnx32)
- [`mesa32`](https://github.com/aks796/mesa32) or a compatible 32-bit Switch Mesa build
- devkitPro / devkitA64 for the 64-bit launcher

The runtime is intended to be kept as a git submodule:

```bash
git submodule update --init --recursive
```

Build the 32-bit game payload from the repository root:

```bash
./build.sh
```

Then build the launcher:

```bash
cd launcher
./build.sh
```

Final release executable:

```text
launcher/spiderman_total_mayhem_nx.nro
```

The launcher embeds `launcher/icon.jpg` as its Homebrew/HOME-menu artwork.

The 32-bit game program is built as an ExeFS NSP and carried inside the 64-bit launcher NRO. AArch32 game code cannot be launched directly as a normal hbloader NRO, so the launcher installs the payload for the forwarder's title and restarts into it.

---

## Technical Notes

Notable Switch-side work includes:

- AArch32 ELF loading and Android/Bionic compatibility.
- ARM/Thumb-aware runtime hooks.
- OpenGL ES 1.1 through Mesa/Nouveau.
- CPU-side S3TC/DXT texture decoding where required.
- 1280×720 screen and UI helpers.
- Native Switch controller-to-touch/gameplay input bridge.
- Game-state-aware controller menu cursor.
- Save/settings path mapping.
- Native OGG/Vorbis decoding and mixing to Switch `audout`.
- Runtime CPU/GPU performance-state handling.

Debug information is written to:

```text
sd:/switch/spiderman_total_mayhem_nx/debug.log
```

If the game crashes, also include `crash.log` when reporting the issue.

---

## Status

**Tested on real Nintendo Switch hardware.**

Working in the current release:

- Rendering
- Gameplay
- Controller input
- Touchscreen
- Menus
- Context/NPC/QTE prompts
- Save data
- Settings persistence
- Music
- Sound effects
- Voice audio
- 720p presentation
- HOME-menu launcher / forwarder workflow

Only **Android 1.0.8 / versionCode 108** is supported by this release. Other Android versions have not been tested and may use different native code or offsets.

---

## Credits

### Nintendo Switch Port

- **[Bora Eskicioğlu](https://github.com/boraeskicioglu)** — Nintendo Switch port, integration, controller/UI work, rendering fixes, audio backend, performance work and hardware testing.

### Runtime and Porting Work

- **[aks796/android32](https://github.com/aks796/android32)** — shared AArch32 Android runtime for Nintendo Switch.
- **Andy Nguyen / TheOfficialFloW** and **fgsfds** — foundational Android `.so` loader work referenced by the runtime.
- **[xerpi/vita2hos](https://github.com/xerpi/vita2hos)** and contributors — AArch32 Horizon OS/toolchain work.
- **[cualquiercosa327/spiderman_vita](https://github.com/cualquiercosa327/spiderman_vita)** — technical reference for Total Mayhem's Android native functions and audio mapping.
- **Sean Barrett / stb** — `stb_vorbis`.
- **devkitPro**, **libnx**, **libnx32**, **Mesa** and **Nouveau** contributors.

### Original Game

**Ultimate Spider-Man: Total Mayhem** was developed/published for mobile by **Gameloft**. Spider-Man and related characters and trademarks belong to their respective rights holders, including Marvel.

---

## Contributing / Bug Reports

When reporting a problem, please include the exact action/level plus `debug.log` and `crash.log` if generated. Do **not** upload copyrighted APKs or game-data files to GitHub issues.

---

## Legal / Disclaimer

This is an unofficial fan-made compatibility port and is not affiliated with, sponsored by or endorsed by Nintendo, Gameloft, Marvel or any other rights holder.

The repository and release packages must not contain the original APK, `libspiderman.so`, levels, music, voice files, textures or other runtime game data. Users must provide their own legally obtained copy of the game.

The Homebrew Menu artwork remains the property of its respective rights holders and is used only to identify the game.

The Switch-specific source written for this port is distributed under the terms described in [LICENSE](LICENSE). Third-party components retain their own licenses; see [THIRD_PARTY.md](THIRD_PARTY.md).

---

## Version

Current release: **v1.0.0**.
