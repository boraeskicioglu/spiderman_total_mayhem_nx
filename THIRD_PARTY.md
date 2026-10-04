# Third-Party Components

This repository depends on or references software maintained by other projects. Their original copyright notices and license terms remain in force.

## android32

- Project: https://github.com/aks796/android32
- License: MIT
- Copyright: aks796
- Includes portions derived from work by Andy Nguyen (TheOfficialFloW), fgsfds and libnx contributors under their stated upstream terms.

The recommended repository layout keeps `android32` in `runtime/` as a git submodule.

## libnx32

- Project: https://github.com/aks796/libnx32
- Purpose: 32-bit Nintendo Switch / Horizon OS support.

## Mesa / Nouveau

Graphics are provided by the 32-bit Switch Mesa/Nouveau stack used by the `android32` runtime. Those components retain their upstream licenses.

## stb_vorbis

- Project: https://github.com/nothings/stb
- Author: Sean Barrett and contributors.
- Used by the native Switch audio backend to decode OGG/Vorbis files supplied by the user.

## spiderman_vita

- Project: https://github.com/cualquiercosa327/spiderman_vita
- Used as a technical reference for the Android version's native function behavior and sound-ID mapping.

## Original Game

Ultimate Spider-Man: Total Mayhem is proprietary software. Its APK, `libspiderman.so`, levels, audio, video, textures and other assets are not part of the open-source license for this port and must not be distributed in the source repository or release package.
