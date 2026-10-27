# HiBy R1 hardware and packaging audit

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
2. Match AIC8800 drivers/firmware and transport startup to a compatible kernel;
   validate Wi-Fi, Bluetooth, simultaneous use, suspend/resume and wake.
3. Measure RAM during local playback, decoding, browsing and downloads. Host
   RSS includes x86-64 SDL and host libraries and is not a target memory budget.
4. Validate the reconstructed squashfs, model metadata, every module's vermagic,
   OTA chunk chain and final image sizes against measured partitions.
5. Test stock recovery on the matching hardware and keep that exact stock image.

No hardware has been flashed. A user assumption of “latest” is not evidence of
hardware identity. The original request explicitly requires approval to flash.

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
