# Uzem browser integration

## Selected source

The core forks https://github.com/libretro/libretro-uzem at
`d991ee94547c8294abc1c4cb73d63116aa58b5bc` (upstream commit 2026-08-23).
At evaluation on 2026-09-12 it had 8 stars. It supplies an existing libretro ABI,
AVR CPU/video/audio emulation and cartridge loading, so it fits the existing
EmulatorJS Provider without adding another host integration.

https://github.com/Uzebox/uzebox (152 stars, latest commit 2026-08-24 at evaluation)
is the official hardware/kernel/game/tool repository. Its AVR game kernel is
not itself a browser emulator. It supplies the public Arkanoid test cartridge.
https://github.com/Jubatian/cuzebox (14 stars, latest commit 2020-08-30 at evaluation)
was considered for emulation completeness, but its standalone frontend and lack
of complete instant snapshots require additional integration. Stars alone did
not determine selection. Compatibility beyond the tested game remains unproven.

## Build and provenance

`retrom-fork.json` pins upstream, EmulatorJS RetroArch linker and public ABI.
The recipe pins emsdk by image digest, builds and runs the native state regression,
then builds single-threaded Asyncify WASM and a 7z EmulatorJS `.data` asset.
The candidate descriptor captures the exact Git working-tree digest, original
commit, dirty flag and hashes/sizes of the core, source archive and license.
The source archive contains this fork's exact build recipe; RetroArch sources
are obtained from the pinned public commit. LICENSE in the artifact concatenates
Uzem MIT and the linker's GPL text. Component headers retain their own notices.

Run `.github/rpg-runtime/test-native.sh` for timer, EEPROM, controller latch,
corruption/truncation rejection and absent-SD behavior. Supply a local `.uze`
path to additionally compare complete states and all 60 frames of replay both
within one instance and after destroy/recreate. No script downloads games.
Use `.github/rpg-runtime/build-candidate.sh /absolute/empty/output` or Retrom's
`make pfb-core-build PFB=<flow> CORE=uzem` for explicit candidate builds.

## Contract and limits

The target accepts version-1 ATmega644 `.uze` standalone cartridges. It does not
mount SD media, exposes no mouse and polls player one. The SNES-compatible
controller layout maps directly to native libretro inputs. Firmware may probe
SPI; an absent card returns `0xff` without entering an SD response state.

State v1 has a 20-byte little-endian header (magic `RUZ1`, version, length,
cartridge FNV-1a identity, payload FNV-1a checksum). Fields are explicitly encoded
as 32-bit little-endian values, with 810336 bytes total. The identity/checksum
are consistency checks, not cryptographic authentication. CPU, RAM, mutable
flash, EEPROM, timing, scanline, framebuffer, RNG and latched input are restored;
decoded instructions are reconstructed. No host pointers enter the format.
Public gzip compression belongs to the runtime Provider, not this serializer.

`ACC-UZEBOX-001` in Retrom owns import, review preview, product start, standard
controller movement/confirm, screenshot, save upload and fresh Launch restore.
The maintained test compares the entire saved game frame and requires further
input after restore. Browser automation uses a virtual standard controller;
it must not be reported as a physical controller or broad game compatibility test.

## Release promotion

The integration supports explicit local candidates and immutable releases. Promote only after the
core checks and actual product case pass, review and authorized merge into the
maintenance branch. The release workflow validates an annotated tag on the maintenance branch, runs
`.github/rpg-runtime/build-release.py`, and publishes an immutable core tag and exact
`rpg-runtime-release.json` metadata; then replace development inputs in
retrom-runtime with those released assets, run its release gates, and separately
pin the resulting Provider in Retrom. Do not place candidate hashes in production
locks or describe a local candidate as a published release.
