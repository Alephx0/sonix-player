#!/usr/bin/env python3
"""Read-only validation of an extracted HiBy OTA chunk chain and whole images."""
import argparse
import hashlib
from pathlib import Path

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('extracted', type=Path)
args = parser.parse_args()

def require(condition, message):
    if not condition:
        raise SystemExit('FAIL: ' + message)

ota = args.extracted / 'ota_v0'
manifest = (ota / 'ota_update.in').read_text()
images = []
for block in manifest.split('img_type=')[1:]:
    fields = dict(line.split('=', 1) for line in block.splitlines() if '=' in line)
    images.append(fields)
if not images:
    raise SystemExit('No image entries')
for entry in images:
    name, expected = entry['img_name'], entry['img_md5']
    if name not in ('xImage', 'rootfs.squashfs'):
        raise SystemExit('Unexpected image name: ' + name)
    chunks = sorted(ota.glob(name + '.[0-9][0-9][0-9][0-9].*'))
    listed = (ota / ('ota_md5_' + name + '.' + expected)).read_text().split()
    require(len(chunks) == len(listed) > 0, 'Missing/extra chunks')
    whole = hashlib.md5()
    previous, total = expected, 0
    for number, chunk in enumerate(chunks):
        require(chunk.name == f'{name}.{number:04d}.{previous}', 'Broken chunk name chain')
        data = chunk.read_bytes()
        require(0 < len(data) <= 524288, 'Bad chunk size')
        digest = hashlib.md5(data).hexdigest()
        require(digest == listed[number], 'Chunk digest mismatch')
        whole.update(data)
        total += len(data)
        previous = digest
    require(total == int(entry['img_size']), 'Whole-image size mismatch')
    require(whole.hexdigest() == expected, 'Whole-image digest mismatch')
    print(f'PASS {name}: {len(chunks)} chunks, {total} bytes, MD5 {expected}')
print('Integrity only: this does not establish image authenticity or board compatibility.')
