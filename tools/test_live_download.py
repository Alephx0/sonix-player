#!/usr/bin/env python3
"""Opt-in real download/index/decode integration. Leaves authorized fixture locally."""
from pathlib import Path
import subprocess
import sys
import tempfile
import argparse
import time

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('repo', type=Path)
parser.add_argument('--fresh', action='store_true', help='Use a new card root to force a real transfer')
args = parser.parse_args()
repo = args.repo.resolve()
player = repo / 'sonix-player'
root = (repo / 'artifacts' / ('live-' + str(time.time_ns())) / 'sd') if args.fresh else repo / 'artifacts/integrated/sd'
root.mkdir(parents=True, exist_ok=True)
source = Path(__file__).resolve().parents[1] / 'sonix-player/tests/live_download.c'
with tempfile.TemporaryDirectory(prefix='sonix-live-') as temp:
    work = Path(temp)
    objects = list((player / 'build_host').rglob('*.o')) + list((player / 'build_host').rglob('*.opp'))
    objects = [str(p) for p in objects if p != player / 'build_host/main.o']
    subprocess.run(['objcopy', '--redefine-sym', 'main=sonix_application_main',
                    str(player / 'build_host/main.o'), str(work / 'app_main.o')], check=True)
    objects.append(str(work / 'app_main.o'))
    libs = subprocess.check_output(['pkg-config', '--libs', 'sdl2', 'freetype2', 'opusfile', 'wavpack'], text=True).split()
    subprocess.run(['gcc', '-DHOST_BUILD=1', '-I' + str(player), '-c', str(source),
                    '-o', str(work / 'test.o')], check=True)
    subprocess.run(['g++', '-o', str(work / 'test'), str(work / 'test.o'), *objects,
                    *libs, '-lpthread', '-lm', '-ldl', '-lasound'], check=True)
    subprocess.run([str(work / 'test'), str(root)], cwd=player, check=True, timeout=200)
