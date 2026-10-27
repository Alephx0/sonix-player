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
files = subprocess.check_output(['git', 'ls-files', '-z', '--cached', '--others', '--exclude-standard'], cwd=source)
# No --delete: do not erase Linux-local data, toolchains or generated resources.
subprocess.run(['rsync', '-a', '--from0', '--files-from=-', str(source) + '/', str(dest) + '/'],
               input=files, check=True)
subprocess.run(['make', args.target, '-j' + str(args.j)], cwd=dest / 'sonix-player', check=True)
