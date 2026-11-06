# HiBy R1 hardware and packaging audit

## 2026-11-06: Waveform/Library update prepared for the owner

The owner requested packaging and SD-card staging so they can initiate flashing.
Package: `artifacts/package-wave-20261106/r1.upt` (48,064,512 bytes), SHA-256
`38012cd5b2aacaf871b2532d82702fdb41f937fd8adbcb4ec931da5e14f88e18`.
It contains the exact R1-tested player, MD5 `bd35eadd4e14a39541e9c614a26fae0d`.

Read back the connected device's installed kernel and rootfs and matched their
image contents to the previously flashed package. Repackaged that known base,
changing only `usr/bin/sonix_player` and the system-info build stamp. Kernel,
drivers, scripts, permissions, ownership, and symlinks remain unchanged.
Verified the final ISO by re-extracting, checking every OTA chunk/chain and whole
image digest, and comparing the reconstructed rootfs contents and metadata.
Rootfs: 44,703,744 bytes, below measured 47,185,920-byte `mtd2` capacity.
Kernel: 2,940,992 bytes, unchanged. This package is for this confirmed R1 setup;
it does not establish compatibility with other board/radio revisions.

Staging/report details are in `artifacts/package-wave-20261106/package-report.json`.
The intended card path is `/mnt/sd_0/r1.upt`; the previous update is preserved at
`/mnt/sd_0/.local/package-wave-20261106-backup/r1-previous.upt`.
The report records the final on-card SHA-256 verification. **No flash or reboot
was initiated.** The owner can use Settings -> Device -> System -> Update from
SD card. Previous records below describe earlier builds and historical blockers.

Audit date: 2026-10-27. User selected latest model/firmware as the target; no
physical device revision has been read. This is not a verified flash release.

## Evidence

The [official specification](https://guide.hiby.com/en/docs/products/audio_player/hiby_r1/spec/)
lists Ingenic X1600E, 64 MB RAM, 1 Gbit flash, 480 × 800 display, AXP2101,
CS43131, 2.4 GHz Wi-Fi, Bluetooth 5.0, microSD up to 2 TB, 1150 mAh battery,
and Linux 4.4.94. Total flash capacity is not free rootfs capacity.

Sonix baseline: `fe94fe796d93c6efa50d1107a734864c9c6a03d5`. Its model table
selects R1 based on `system-info.json`; its R1 overlay identifies OS 1.3.0.
Its toolchain uses little-endian MIPS32r2/o32 and checks dynamic symbols against
glibc 2.22. Those flags and ABI validation are retained.

Stock 2.0 was downloaded from the HiBy CDN URL recorded in
[the firmware research repository](https://github.com/bidhata/Hiby-R1-Mod/blob/main/stock_2.0_download.txt),
then independently unpacked using 7z and unsquashfs. No stock code was executed.
Local archive: `artifacts/stock/r1-stock-2.0.upt` (ignored, not redistributed).

- Size: 44,255,232 bytes.
- SHA-256: `0fec88bb3ad0baa55a703e4e2b1038560d4a526f0dd024d2ed037f1560ed09f9`.
- OTA compatibility: `hiby_r1_x1600_auto_rf_auto`.
- Kernel: 3,731,520 bytes; manifest MD5 `c446c0a3b770674ea13bf8e42f63be7e`.
- Rootfs: 40,103,936 bytes; manifest MD5 `f50a0ca01ddf12d789668261c1af2f77`.
- Read-only validation checked all 8 kernel and 77 rootfs chunks, every chain
  filename, each chunk digest, both assembled sizes and both whole-image digests.
  Reproduce with `tools/verify_stock_chunks.py`; this checks integrity, not origin.
- Rootfs contains `module_driver/r1_wireless_select.sh`, which reads
  `axp2101/parameters/detected_soc_id` and `detected_soc_id_valid`.

| SoC ID in stock selector | SoC name in selector | Wireless branch |
|---|---|---|
| `0x0a00` / 2560 | X1600E | Broadcom, `cywdhd.r1` |
| `0x4a00` / 18944 | X1600EN | AIC8800D40, `aic8800_bsp.r1` |
| `0x4900` / 18688 | X1600HN | AIC8800D40, `aic8800_bsp.r1` |

This is direct script evidence, not proof of which board the user owns.

## Release blocker

Sonix's R1 overlay replaces the kernel with `kernel/xImage-R1` (2,940,992
bytes), installs Broadcom replacement modules, and replaces wireless startup
scripts. `src/system/bluetooth/bluetooth.c` hardcodes `brcm_patchram_plus`,
Broadcom firmware and its transport startup. Keeping AIC files from stock is
insufficient: the kernel, module ABI, radio selection, transport and suspend
logic must agree. Copying the current overlay over 2.0 is not validated.

The packer now fails before overlay mutation when stock contains the multi-radio
selector or AIC8800 markers. No bypass is provided. It also limits compressed R1
rootfs size to the stock image size until actual partition limits are measured.
This conservative bound can reject an otherwise fitting image; it cannot prove
hardware compatibility. Existing R3 Pro II packing behavior is unchanged.

## Work needed before a flash candidate

1. Read actual `/proc/cpuinfo`, `/proc/mtd`, `/proc/meminfo`, `/proc/cmdline`,
   loaded modules, kernel version, and AXP2101 SoC ID on the device.
   `tools/r1_readonly_diagnostics.sh` collects these to stdout from an existing
   device shell. It changes no device settings and has only been syntax-checked
   on the host, not executed on an R1.
2. Match AIC8800 drivers/firmware and transport startup to a compatible kernel;
   validate Wi-Fi, Bluetooth, simultaneous use, suspend/resume and wake.
3. Measure RAM during local playback, decoding, browsing and downloads. Host
   RSS includes x86-64 SDL and host libraries and is not a target memory budget.
4. Validate the reconstructed squashfs, model metadata, every module's vermagic,
   OTA chunk chain and final image sizes against measured partitions.
5. Test stock recovery on the matching hardware and keep that exact stock image.

No hardware has been flashed. A user assumption of “latest” is not evidence of
hardware identity. The original request explicitly requires approval to flash.

## 2026-11-05: owner-approved candidate from stock 2.0

The owner accepted the AIC8800 risk and approved one build from stock 2.0
(Wi-Fi and Bluetooth would not work on an AIC board). Findings while packing:

- **CRLF overlay (boot blocker, fixed).** This Windows checkout has
  `core.autocrlf=true`; 38 R1 overlay files (69 across both models) were
  checked out CRLF although the committed blobs are LF, including
  `usr/bin/sonix_player.sh`, `S80_bt_init`, the module list and
  `system-info.json`. `#!/bin/sh\r` cannot be executed, so the player would
  never start; the stale `\r` is also why `sonix_launch` was reported as "0
  lines". Fixes: `sonix-packer/assets/.gitattributes` (`* -text`) for future
  checkouts, and the packer now dies on any `#!` script containing CR. The
  existing worktree copies still need a re-checkout; the candidate was packed
  from `git archive HEAD` with LF.
- **Size.** Rootfs is 46,804,992 bytes with stock 2.0's AIC8800 driver and
  firmware left in, 44,675,072 without them. Neither fits the stock-size bound
  (40,103,936). Upstream releases ship larger R1 rootfs images: 1.2.0 45,334,528
  and 1.3.0 44,638,208 (downloaded and measured 2026-11-05). Neither kernel
  carries the partition table (the bootloader passes `mtdparts`), so the real
  limit is still unmeasured.
- **Candidate** (working copy only, not in the repo packer): AIC8800 modules
  and firmware dropped, rootfs bound raised to 45,334,528. Its `r1.upt` is
  48,035,840 bytes. It was verified independently:
  - The chunk chain and manifest match stock 2.0's format.
  - The kernel is byte-identical to upstream 1.3.0's.
  - `sonix_player` is the current build.
  - The launcher execs `sonix_launch`, and no script contains CR.
  - The compatibility string is unchanged.

  The owner approved the raised bound. On 2026-11-05 the candidate was copied
  to the microSD root as `r1.upt`, and its SHA-256 on the card matches
  (`ff3635168987…a493`). The owner flashed it the same day: it installed and
  booted without issues (owner report); Wi-Fi/Bluetooth not yet reported.

- **Second image (2026-11-05, later):** the freeze, scan and display size fixes
  (UI_DECISIONS 2026-11-05 device). Same recipe; assets are now taken from the
  worktree with every file git holds as LF but the checkout holds as CRLF put
  back to LF in the copy (69 files), and `sonix-packer/assets/.gitattributes`
  is `* text=auto eol=lf` so future checkouts are LF without git reporting the
  current CRLF copies as changed. Rootfs 44,679,168 bytes; r1.upt 48,039,936
  bytes, SHA-256 `c6f032c5…a66839`; independent verification passed (format,
  kernel, current player build, launcher, no CR in any script).

## Installation and recovery status

Do not install this development checkout as firmware yet. Upstream documents
copying the matching `.upt` to the microSD root, charging above 30%, and selecting
the device's SD firmware-update command. Its recovery updater does not validate
the model. Never substitute `r3proii.upt` for `r1.upt` or rename one to the other.

Upstream documents returning to stock by flashing the original matching HiBy
image through that same workflow. That is a documented route, not a recovery
test performed here; a boot failure may prevent reaching the menu. Latest-board
recovery and bootloader-level recovery remain unverified. Preserve stock 2.0
and diagnostics before planning any hardware trial.

### 2026-11-05: Connected-device confirmation and temporary test

ADB now connected as serial `dev` (the stock gadget reports Nexus_4/mako;
kernel and device files identify the R1). Kernel 4.4.94-sonix. Input `hyn_ts`
is /dev/input/event1, direct-touch. Live EVIOCGABS probe: legacy ABS_X/Y both
succeed but return min=max=0; MT X is 0..720 and MT Y is 0..1280. This confirms
the calibration defect on the connected hardware, rather than merely a theory.
The startup log confirms Large layout: 384x640 drawn to a 480x800 framebuffer.

Transferred the built player to /tmp/sonix_player_touchfix, verified its MD5,
stopped the old reboot supervisor before stopping the player, and launched a
new supervisor with this temporary executable. PID 2070 at test start. The new
log explicitly reports `(0,0)-(720,1280) -> 384x640 logical pixels`.
Firmware is unchanged. Reboot discards /tmp and restores the flashed player.
Owner asked to check center/edge taps and collection scrolling; response pending.

Prepared, but did not flash or copy onto the card, a package at
artifacts/touch-calibration-20261105/r1.upt. SHA-256:
27875db179c43a4982cdf110bd0f2f139b82d09bc81c09d1842761f8846bd65d
Uses the previous device candidate's kernel/assets/packing recipe plus the
corrected binary. Independently verified OTA chains, extracted player hash,
unchanged kernel, LF scripts, and rootfs 44,683,264 bytes < measured rootfs
partition 47,185,920 bytes (/proc/mtd: mtd2 02d00000). This does not prove recovery
or alternate-slot capacity (rootfs2 is smaller); keep prior recovery limitations.
Evidence: artifacts/touch-calibration-20261105/device-live.log and
package-report.json. Explicit user authorization remains required for flashing.
