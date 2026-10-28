#!/usr/bin/env python3
"""Create original silent fixtures, validate real APIs, retain a simulator card."""
from pathlib import Path
import json
import subprocess
import struct
import sys
import tempfile
import wave

repo=Path(sys.argv[1]).resolve()
player=repo/'sonix-player'
card=Path(tempfile.mkdtemp(prefix='collection-fixture-',dir=repo/'artifacts'))/'sd'
(card/'Music').mkdir(parents=True,exist_ok=True)
(card/'.local').mkdir(exist_ok=True)
titles=['Night Walk','Café Lights','Blue Hour','First Light','A Quiet Morning']
for i,title in enumerate(titles):
    path=card/f'Music/track{i}.wav'
    with wave.open(str(path),'wb') as wav:
        wav.setparams((1,2,44100,0,'NONE','not compressed'))
        for _ in range(30): wav.writeframes(b'\0\0'*44100)
    info=b'INFO'+b'IGNR'+struct.pack('<I',8)+b'Ambient\0'
    with path.open('r+b') as stream:
        stream.seek(0,2)
        stream.write(b'LIST'+struct.pack('<I',len(info))+info)
        size=stream.tell()
        stream.seek(4);stream.write(struct.pack('<I',size-8))
    path.with_suffix('.wav.source.json').write_text(json.dumps({'bytes':path.stat().st_size,'title':title,
        'artist':'Night Ensemble','album':'Night Lights'}))
    path.with_suffix('.wav.tags').write_text('year=1995\n'+('artist=Herta FLEBBE - Sopran, Frauke HAASEMANN - Alt, Wilhelm KAISER - Tenor, Paul GUMMER - Bass\nalbum_artist=Herta FLEBBE - Sopran, Frauke HAASEMANN - Alt, Wilhelm KAISER - Tenor, Paul GUMMER - Bass\n' if i==0 else ''))
    path.with_suffix('.lrc').write_text('[00:00.00]An original line at the start\n[00:04.00]We walk into the night\n[00:10.00]A quiet light along the way\n[00:20.00]The morning is almost here\n')
subprocess.run([sys.executable,str(repo/'tools/make_artwork_fixture.py'),str(card/'Music')],check=True)
with tempfile.TemporaryDirectory(prefix='sonix-offline-') as temp:
    work=Path(temp)
    objects=list((player/'build_host').rglob('*.o'))+list((player/'build_host').rglob('*.opp'))
    objects=[str(p) for p in objects if p!=player/'build_host/main.o']
    subprocess.run(['objcopy','--redefine-sym','main=sonix_application_main',str(player/'build_host/main.o'),str(work/'app_main.o')],check=True)
    objects.append(str(work/'app_main.o'))
    libs=subprocess.check_output(['pkg-config','--libs','sdl2','freetype2','opusfile','wavpack'],text=True).split()
    subprocess.run(['gcc','-DHOST_BUILD=1','-I'+str(player),'-c',str(player/'tests/collection_library.c'),'-o',str(work/'test.o')],check=True)
    subprocess.run(['g++','-o',str(work/'test'),str(work/'test.o'),*objects,*libs,'-lpthread','-lm','-ldl','-lasound'],check=True)
    subprocess.run([str(work/'test'),str(card)],cwd=player,check=True,timeout=60)
print('Fixture card:',card)
