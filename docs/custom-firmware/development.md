# Building and maintaining this branch

## Environment

Development used Ubuntu 24.04 under WSL, GCC 13 host tools, and the repository's
Rockbox GCC 9.5 MIPS toolchain. Use a Linux checkout at a path **without spaces**:
upstream Makefile include/object rules do not quote whitespace paths throughout.
The Windows workspace remains the authoritative editable checkout.

Install host/build dependencies in Ubuntu:

```sh
sudo apt-get install build-essential git pkg-config libsdl2-dev libfreetype6-dev \
  libopusfile-dev libwavpack-dev libasound2-dev libcurl4-openssl-dev \
  wget curl texinfo bison flex python3 python3-venv autoconf automake libtool-bin \
  libgmp-dev libmpfr-dev libmpc-dev rsync xvfb xauth xdotool imagemagick \
  p7zip-full squashfs-tools genisoimage
git clone https://github.com/Alephx0/sonix-player.git ~/sonix-r1-build
```

For this Windows workspace, run inside WSL:

```sh
python3 '/mnt/c/Users/justi/Documents/Python/Hiby R1 Spotify/tools/build_wsl.py' \
  --build-dir /home/justi/sonix-r1-build host
cd /home/justi/sonix-r1-build/sonix-player
./rockboxdev/rockboxdev.sh
python3 '/mnt/c/Users/justi/Documents/Python/Hiby R1 Spotify/tools/build_wsl.py' \
  --build-dir /home/justi/sonix-r1-build target
```

`build_wsl.py` copies tracked and non-ignored new sources using rsync, without
deleting Linux build caches, then invokes `make host/target -j8`. A native Linux
checkout can directly run those make commands in `sonix-player/`. After deleting
or renaming source files, remove the corresponding stale copied files/objects or
use a fresh build checkout; the sync helper deliberately does not delete files.
LVGL is pinned by upstream to 9.6.0. Keep the target's glibc-compat include and
post-link symbol guard. Never weaken the ABI check to get a successful build.

Target outputs are `sonix_player`, `sonix_player_debug` and `sonix_launch` in
the Linux player directory. Keep the unstripped binary with any deployed build
for symbolized diagnostics. A target executable alone is not a firmware image.

## Verification commands

From the synchronized Linux checkout:

```sh
bash sonix-player/tests/discovery/run.sh
python3 tools/test_library_import.py "$PWD"
python3 tools/test_r1_packaging.py
python3 tools/test_live_download.py "$PWD" --fresh
python3 tools/host_smoke.py "$PWD" --online
```

The first command enables ASan/UBSan by default and uses deterministic queue
fixtures plus a temporary local TLS server. `LIVE=1` additionally exercises live
catalog search. `test_live_download.py` contacts Archive and stores an openly
licensed fixture under `artifacts/integrated/sd`. It uses the actual HTTPS, queue,
checksum, metadata index and decoder implementations. It does not emit DAC audio.
Do not run the live integration and simulator concurrently against the same SD root.

`host_smoke.py` starts an authenticated private Xvfb TCP display (:99), uses the
software SDL renderer and captures actual LVGL windows. It sets a private config
and SD root, disables host mounting, and terminates its simulator/X server.
Use another display if :99 is already occupied. Its optional online scenario uses
touch coordinates at 480 × 800 and fixed waits; review the resulting images,
because timing or source ordering can change. PNGs and metrics appear under
`artifacts/integrated`; selected evidence is copied into this documentation.

## Packaging, updating and recovery

Read [hardware-and-packaging.md](hardware-and-packaging.md) before using the packer.
Stock 2.0 is intentionally rejected by the current R1 compatibility guard because
the overlay's kernel/radio stack is unverified for its AIC boards. Do not remove
that guard or rename a different model's update file. No flash/recovery operation
was performed during development.

The R1 overlay's OTA repository points to this fork so future updates do not
silently replace its modifications with upstream binaries. No fork firmware
release exists yet, so an OTA lookup cannot supply this development build.
The source, dependencies, exact stock baseline, checksums and matching debug
binary must accompany a future tested release, subject to their licences.

## Maintenance boundaries

- Fetch upstream, review `FEATURES.md`, regenerate the baseline inventory only
  when intentionally changing baseline, and reconcile every new capability.
- Keep shell/navigation changes distinct from library and transport behavior.
  Preserve the six Tools routes and all existing audio/streaming entry points.
- Extend capability bits and native adapter validation together. Bound responses,
  result counts and concurrency; never execute untrusted provider scripts.
- Any storage change must rerun interrupted transfer, no-replace, checksum,
  insufficient-space, removal, restored-queue and real-library tests.
- Any target/system change requires the ABI check and physical radio, USB,
  suspend, audio, filesystem and recovery checks before a flash candidate.
- Review new UI strings for translation and long-text layouts before release.

Known reliability limits include source-side metadata/licence accuracy, no
provider pagination, finite retry policy, sidecar metadata instead of embedded
retagging, unmeasured audio scheduling under actual hardware load, and queue
shutdown waiting on connection establishment. See the feature matrix for scope.
