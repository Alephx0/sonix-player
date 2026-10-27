#!/bin/bash
# Source-only checks shared by the packer and regression tests. They perform
# no writes and deliberately offer no bypass for unverified radio hardware.
r1_compatibility_check() {
    local stock_root="$1" overlay="$2"
    if [ -e "$stock_root/module_driver/r1_wireless_select.sh" ] ||
       [ -e "$stock_root/module_driver/aic8800_fdrv.ko" ] ||
       [ -e "$stock_root/etc/init.d/S52wifi_aic_init_config" ]; then
        echo "R1 packaging blocked: this stock firmware supports AIC8800 boards." >&2
        echo "The Sonix kernel overlay and Bluetooth startup are Broadcom-only." >&2
        echo "Validate AIC8800 kernel/modules, radio selection and suspend first." >&2
        return 1
    fi
    if ! grep -Eq '"device-name"[[:space:]]*:[[:space:]]*"HiBy R1"' \
        "$overlay/usr/resource/sonix/components/system-info.json"; then
        echo "R1 packaging blocked: overlay does not identify HiBy R1." >&2
        return 1
    fi
}

r1_rootfs_size_check() {
    local size="$1" stock_size="$2"
    # Until /proc/mtd has been measured, the original compressed image size
    # is a conservative upper bound, not a claim about partition capacity.
    [[ "$size" =~ ^[0-9]+$ && "$stock_size" =~ ^[0-9]+$ ]] || return 1
    if [ "$stock_size" -eq 0 ] || [ "$size" -gt "$stock_size" ]; then
        echo "R1 rootfs exceeds the known stock image size; partition measurement required." >&2
        return 1
    fi
}
