#!/usr/bin/env python3
"""Exercise embedded metadata and the native bounded discography query."""
from pathlib import Path
import subprocess,sys,tempfile,struct
repo=Path(sys.argv[1]).resolve();player=repo/'sonix-player'
card=Path(tempfile.mkdtemp(prefix='discography-fixture-',dir=repo/'artifacts'))/'sd';(card/'Music').mkdir(parents=True,exist_ok=True);(card/'.local').mkdir(exist_ok=True)
# Original silence, deliberately varied dates/types/credits. No network catalog.
fixtures=[('Earlier','2024-01-10','album','Nova','Nova',''),('Later','2024-11-02','album','Nova','Nova',''),
('Short','2025-02-01','single','Nova','Nova',''),('Untyped','2023','','Nova','Nova',''),
('Guest','2026-06-03','ep','Other','Other; Nova','Other; Nova'),
('Ambiguous','2026-07-01','album','Other','Nova','')]
for i,(album,date,kind,owner,performer,credits) in enumerate(fixtures):
    args=['ffmpeg','-v','error','-y','-f','lavfi','-i','anullsrc=r=44100:cl=mono','-t','30','-c:a','flac']
    tags={'title':album+' Track','album':album,'date':date,'releasetype':kind,'albumartist':owner,'artist':performer,'artists':credits}
    for k,v in tags.items():
        if v:args+=['-metadata',k+'='+v]
    subprocess.run(args+[str(card/f'Music/release{i}.flac')],check=True)
# A real ID3v2.4 tag tests multiple null-separated values and Picard tag names.
def sync(n):return bytes([(n>>21)&127,(n>>14)&127,(n>>7)&127,n&127])
def frame(k,v):
    b=b'\x03'+v.encode();return k.encode()+sync(len(b))+b'\0\0'+b
body=frame('TPE1','Nova')+frame('TPE2','Alias')+frame('TALB','Identity')+frame('TDRC','2025-08-19')
body+=frame('TXXX','MusicBrainz Album Type\0ep')+frame('TXXX','ARTISTS\0Nova\0Guest')
body+=frame('TXXX','MusicBrainz Artist Id\0id-nova\0id-guest')+frame('TXXX','MusicBrainz Album Artist Id\0id-nova')
raw=card/'audio.mp3'
subprocess.run(['ffmpeg','-v','error','-y','-f','lavfi','-i','anullsrc=r=44100:cl=mono','-t','1','-c:a','libmp3lame','-id3v2_version','0',str(raw)],check=True)
(card/'identity.mp3').write_bytes(b'ID3\x04\0\0'+sync(len(body))+body+raw.read_bytes())
with tempfile.TemporaryDirectory(prefix='sonix-discography-') as temp:
    work=Path(temp)
    objects=list((player/'build_host').rglob('*.o'))+list((player/'build_host').rglob('*.opp'))
    objects=[str(p) for p in objects if p!=player/'build_host/main.o']
    subprocess.run(['objcopy','--redefine-sym','main=sonix_application_main',str(player/'build_host/main.o'),str(work/'app_main.o')],check=True)
    libs=subprocess.check_output(['pkg-config','--libs','sdl2','freetype2','opusfile','wavpack'],text=True).split()
    subprocess.run(['gcc','-DHOST_BUILD=1','-I'+str(player),'-c',str(player/'tests/discography.c'),'-o',str(work/'test.o')],check=True)
    subprocess.run(['g++','-o',str(work/'test'),str(work/'test.o'),*objects,str(work/'app_main.o'),*libs,'-lpthread','-lm','-ldl','-lasound'],check=True)
    subprocess.run([str(work/'test'),str(card)],cwd=player,check=True,timeout=60)
print('Discography fixture:',card)
