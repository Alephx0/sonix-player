#!/usr/bin/env python3
"""Exercise the real playback queue linked with host application objects."""
from pathlib import Path
import subprocess
import sys
import tempfile

repo = Path(sys.argv[1]).resolve()
player = repo / 'sonix-player'
with tempfile.TemporaryDirectory(prefix='sonix-priority-') as temp:
    work = Path(temp)
    objects = list((player / 'build_host').rglob('*.o')) + list((player / 'build_host').rglob('*.opp'))
    objects = [str(p) for p in objects if p != player / 'build_host/main.o']
    subprocess.run(['objcopy', '--redefine-sym', 'main=sonix_application_main',
                    str(player / 'build_host/main.o'), str(work / 'app_main.o')], check=True)
    objects.append(str(work / 'app_main.o'))
    libs = subprocess.check_output(['pkg-config', '--libs', 'sdl2', 'freetype2', 'opusfile', 'wavpack'], text=True).split()
    source = player / 'tests/priority_queue.c'
    subprocess.run(['gcc', '-DHOST_BUILD=1', '-I' + str(player), '-c', str(source), '-o', str(work / 'test.o')], check=True)
    subprocess.run(['g++', '-o', str(work / 'test'), str(work / 'test.o'), *objects,
                    *libs, '-lpthread', '-lm', '-ldl', '-lasound'], check=True)
    subprocess.run([str(work / 'test'), str(work)], cwd=player, check=True, timeout=30)
