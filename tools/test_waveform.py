#!/usr/bin/env python3
"""Native waveform prewarming and restart cache regression using real FLAC."""
from pathlib import Path
import subprocess, sys, tempfile
repo=Path(sys.argv[1]).resolve(); player=repo/'sonix-player'
with tempfile.TemporaryDirectory(prefix='waveform-', dir=repo/'artifacts') as tmp:
    work=Path(tmp); card=work/'sd'; card.mkdir()
    for name,freq in [('a',440),('b',880)]:
        subprocess.run(['ffmpeg','-v','error','-f','lavfi','-i',f'sine=frequency={freq}:duration=2',
                        '-c:a','flac',str(card/(name+'.flac'))],check=True)
    objects=[str(p) for ext in ('*.o','*.opp') for p in (player/'build_host').rglob(ext)
             if p!=player/'build_host/main.o']
    subprocess.run(['objcopy','--redefine-sym','main=sonix_application_main',str(player/'build_host/main.o'),str(work/'app.o')],check=True)
    libs=subprocess.check_output(['pkg-config','--libs','sdl2','freetype2','opusfile','wavpack'],text=True).split()
    libs.insert(0,str(player/'.deps/webp/host/libwebpdecoder.a'))
    subprocess.run(['gcc','-DHOST_BUILD=1','-I'+str(player),'-c',str(player/'tests/waveform.c'),'-o',str(work/'test.o')],check=True)
    subprocess.run(['g++','-o',str(work/'test'),str(work/'test.o'),*objects,str(work/'app.o'),*libs,
                    '-lpthread','-lm','-ldl','-lasound','-Wl,--wrap=audio_quiet_ms'],check=True)
    for phase in ('prepare','restart'):
        subprocess.run([str(work/'test'),str(card),phase],check=True,timeout=20)
