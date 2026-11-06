#!/usr/bin/env python3
"""Exercise local codecs after removal of progressive transfer readers."""
from pathlib import Path
import subprocess
import sys
import tempfile

repo = Path(sys.argv[1]).resolve()
player = repo / 'sonix-player'
with tempfile.TemporaryDirectory(prefix='sonix-local-decode-') as temp:
    work = Path(temp)
    formats=[('lossless.flac',['-c:a','flac','-sample_fmt','s32']),('cbr.mp3',['-c:a','libmp3lame','-b:a','192k']),('cbr-no-header.mp3',['-c:a','libmp3lame','-b:a','192k','-write_xing','0']),('vbr.mp3',['-c:a','libmp3lame','-q:a','3'])]
    for name,options in formats:
        subprocess.run(['ffmpeg','-v','error','-f','lavfi','-i','sine=frequency=440:sample_rate=48000:duration=4','-ac','2',*options,str(work/name)],check=True)
    objects = list((player / 'build_host').rglob('*.o')) + list((player / 'build_host').rglob('*.opp'))
    objects = [str(p) for p in objects if p != player / 'build_host/main.o']
    subprocess.run(['objcopy', '--redefine-sym', 'main=sonix_application_main',
                    str(player / 'build_host/main.o'), str(work / 'app_main.o')], check=True)
    objects.append(str(work / 'app_main.o'))
    libs = subprocess.check_output(['pkg-config', '--libs', 'sdl2', 'freetype2', 'opusfile', 'wavpack'], text=True).split()
    libs.insert(0, str(player / ".deps/webp/host/libwebpdecoder.a"))
    source = player / 'tests/local_decode.c'
    subprocess.run(['gcc', '-DHOST_BUILD=1', '-I' + str(player), '-c', str(source), '-o', str(work / 'test.o')], check=True)
    subprocess.run(['g++', '-o', str(work / 'test'), str(work / 'test.o'), *objects,
                    *libs, '-lpthread', '-lm', '-ldl', '-lasound'], check=True)
    subprocess.run([str(work / 'test'), str(work)], cwd=player, check=True, timeout=30)
