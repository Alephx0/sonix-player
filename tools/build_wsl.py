#!/usr/bin/env python3
"""Sync tracked project sources into a Linux path without spaces, then build.

Run in WSL: python3 tools/build_wsl.py --build-dir /home/justi/sonix-r1-build host
The source workspace stays authoritative; compiler caches stay on Linux ext4.
"""
import argparse
from pathlib import Path
import subprocess

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--build-dir', type=Path, required=True)
parser.add_argument('target', choices=['host', 'target'])
parser.add_argument('-j', type=int, default=8)
args = parser.parse_args()
source = Path(__file__).resolve().parents[1]
dest = args.build_dir.resolve()
if ' ' in str(dest) or dest == source or not (dest / '.git').exists():
    raise SystemExit('Use a separate existing Linux Git checkout at a path without spaces')
listed = subprocess.check_output(['git', 'ls-files', '-z', '--cached', '--others', '--exclude-standard'], cwd=source)
names = {name.decode() for name in listed.split(b'\0') if name}
files = b'\0'.join(name.encode() for name in sorted(names) if (source / name).is_file()) + b'\0'
# Mirror source removals too: otherwise deleted providers are still compiled by
# Makefile's find src. Limit cleanup to source/test files, never toolchains,
# resources, build caches or music. Resolve both roots before any deletion.
for relative in ('sonix-player/src', 'sonix-player/tests'):
    managed = (dest / relative).resolve()
    if not managed.is_relative_to(dest) or managed.is_symlink():
        raise SystemExit('Unsafe source mirror path')
    for path in managed.rglob('*'):
        if path.is_file() and path.suffix in ('.c', '.h', '.cpp', '.py', '.sh'):
            name = path.relative_to(dest).as_posix()
            if not (source / name).is_file():
                if not path.resolve().is_relative_to(managed):
                    raise SystemExit('Unsafe stale source path')
                path.unlink()
for build in ('build_host', 'build_target'):
    cache = dest / 'sonix-player' / build
    if not cache.exists():
        continue
    for obj in cache.rglob('*'):
        if obj.suffix not in ('.o', '.opp', '.d'):
            continue
        rel = obj.relative_to(cache)
        # Build paths mirror src/ without its prefix.
        candidates = [source / 'sonix-player/src' / rel.with_suffix(ext) for ext in ('.c', '.cpp')]
        if not any(p.exists() for p in candidates) and not str(rel).startswith(('lvgl/', '.deps/')):
            if not obj.resolve().is_relative_to(cache.resolve()):
                raise SystemExit('Unsafe stale object path')
            obj.unlink()
helper = dest / 'sonix-player/spotiflac-core'
if helper.exists() and helper.resolve().is_relative_to(dest):
    helper.unlink()
# No broad --delete: keep Linux-local data, toolchains and generated resources.
subprocess.run(['rsync', '-a', '--from0', '--files-from=-', str(source) + '/', str(dest) + '/'],
               input=files, check=True)
subprocess.run(['make', args.target, '-j' + str(args.j)], cwd=dest / 'sonix-player', check=True)
