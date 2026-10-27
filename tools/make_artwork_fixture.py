#!/usr/bin/env python3
"""Generate original geometric test art beside an existing private audio fixture.

This is simulator test data, not album artwork retrieved from a provider.
Requires ImageMagick convert; refuses to replace an existing artwork file.
"""
from pathlib import Path
import subprocess
import sys
import tempfile

folder = Path(sys.argv[1]).resolve()
target = folder / 'cover.png'
if not folder.is_dir() or target.exists():
    raise SystemExit('Expected an existing fixture folder without cover.png')
with tempfile.TemporaryDirectory(prefix='sonix-art-') as temporary:
    ppm = Path(temporary) / 'geometry.ppm'
    size = 480
    data = bytearray()
    for y in range(size):
        for x in range(size):
            radius = ((x - 240) ** 2 + (y - 240) ** 2) ** .5
            band = int(radius / 30) % 2
            if radius < 185:
                data.extend((35, 130 + 65 * band, 110 + 60 * band))
            else:
                data.extend((22 + x // 12, 28 + y // 10, 42 + x // 16))
    ppm.write_bytes(f'P6\n{size} {size}\n255\n'.encode() + data)
    subprocess.run(['convert', str(ppm), str(target)], check=True)
print('Synthetic UI fixture:', target)
