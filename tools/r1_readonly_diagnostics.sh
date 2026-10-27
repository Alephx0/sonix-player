#!/bin/sh
# Run on a device through an already-authorized shell. Reads only; writes to
# stdout so the operator can redirect the report on their host if desired.
set -u
printf 'R1 read-only diagnostic report\n'
uname -a
for item in /proc/cpuinfo /proc/meminfo /proc/mtd /proc/cmdline /proc/modules; do
    printf '\n--- %s ---\n' "$item"
    if [ -r "$item" ]; then cat "$item"; else printf 'Unavailable\n'; fi
done
for item in /sys/module/axp2101/parameters/detected_soc_id \
            /sys/module/axp2101/parameters/detected_soc_id_valid \
            /usr/resource/sonix/components/system-info.json; do
    printf '\n--- %s ---\n' "$item"
    if [ -r "$item" ]; then cat "$item"; else printf 'Unavailable\n'; fi
done
printf '\n--- filesystem space ---\n'
df -k
printf '\n--- mounted filesystems ---\n'
cat /proc/mounts
