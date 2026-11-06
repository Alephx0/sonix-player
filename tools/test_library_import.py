#!/usr/bin/env python3
"""Link against real host objects and test incremental library imports."""
from pathlib import Path
import subprocess
import sys
import tempfile
import wave
import json

repo = Path(sys.argv[1]).resolve()
player = repo / 'sonix-player'
source = Path(__file__).resolve().parents[1] / 'sonix-player/tests/library_import.c'
with tempfile.TemporaryDirectory(prefix='sonix-library-') as temp:
    work = Path(temp)
    artwork = work / 'art'
    artwork.mkdir()
    subprocess.run([sys.executable, str(Path(__file__).with_name('make_artwork_fixture.py')), str(artwork)], check=True)
    for name in ['first', 'second']:
        with wave.open(str(work / (name + '.wav')), 'wb') as wav:
            wav.setparams((1, 2, 44100, 4410, 'NONE', 'not compressed'))
            wav.writeframes(b'\x00\x00' * 4410)
    audio = work / 'first.wav'
    # Embedded tags exercise the same local import path used by card scans.
    tagged=work/'tagged.wav'
    subprocess.run(['ffmpeg','-v','error','-i',str(audio),'-c:a','copy','-metadata','title=An imported title','-metadata','artist=An independent artist',str(tagged)],check=True)
    tagged.replace(audio)
    objects = list((player / 'build_host').rglob('*.o')) + list((player / 'build_host').rglob('*.opp'))
    objects = [str(p) for p in objects if p != player / 'build_host/main.o']
    subprocess.run(['objcopy', '--redefine-sym', 'main=sonix_application_main',
                    str(player / 'build_host/main.o'), str(work / 'app_main.o')], check=True)
    objects.append(str(work / 'app_main.o'))
    libs = subprocess.check_output(['pkg-config', '--libs', 'sdl2', 'freetype2', 'opusfile', 'wavpack'], text=True).split()
    libs.insert(0, str(player / ".deps/webp/host/libwebpdecoder.a"))
    subprocess.run(['gcc', '-DHOST_BUILD=1', '-I' + str(player), '-c', str(source),
                    '-o', str(work / 'test.o')], check=True)
    subprocess.run(['g++', '-o', str(work / 'test'), str(work / 'test.o'), *objects,
                    *libs, '-lpthread', '-lm', '-ldl', '-lasound'], check=True)
    subprocess.run([str(work / 'test'), str(work)], cwd=player, check=True, timeout=30)
