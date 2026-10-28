#!/usr/bin/env python3
"""Seed realistic, original audio/metadata through the actual native library APIs."""
import shutil
import subprocess
import sys
from pathlib import Path

def create(repo, card):
    card=Path(card); music=card/'Music'; music.mkdir(parents=True)
    (card/'.local').mkdir()
    art=card/'Artwork'; art.mkdir()
    subprocess.run([sys.executable,str(repo/'tools/make_artwork_fixture.py'),str(art)],check=True,stdout=subprocess.DEVNULL)
    fixtures=[('Night Walk','Night Lights','2024-11-02','album','Night Ensemble','Night Ensemble'),
              ('Café Lights','Night Lights','2024-11-02','album','Night Ensemble','Night Ensemble; Nova'),
              ('A remarkably long song title that must stay inside its row without hiding the action button','Night Lights','2024-11-02','album','Night Ensemble','Night Ensemble'),
              ('First Light','First Light','2025-02-01','single','Nova','Nova'),
              ('Guest Appearance','A Shared Horizon','2026-03-04','ep','Other','Other; Nova'),
              ('Without a Picture','Unillustrated','1995','album','Quiet Quartet','Quiet Quartet')]
    for i,(title,album,date,kind,owner,artists) in enumerate(fixtures):
        args=['ffmpeg','-v','error','-y','-f','lavfi','-i','anullsrc=r=44100:cl=mono']
        if i<3: args+=['-i',str(art/'cover.png'),'-map','0:a','-map','1:v','-c:v','png','-disposition:v','attached_pic']
        args+=['-t','180','-c:a','flac']
        for k,v in dict(title=title,album=album,date=date,releasetype=kind,albumartist=owner,artist=artists,artists=artists,genre='Ambient').items(): args+=['-metadata',k+'='+v]
        subprocess.run(args+[str(music/f'track{i:03d}.flac')],check=True)
    (music/'track000.lrc').write_text('[00:00.00]An original line at the start\n[00:04.00]We walk into the night\n[00:10.00]A quiet light along the way\n[02:00.00]The morning is almost here\n')
    for i in range(6,257): shutil.copyfile(music/'track005.flac',music/f'track{i:03d}.flac')
    player=repo/'sonix-player'; work=card.parent
    objects=[p for p in (player/'build_host').rglob('*') if p.suffix in ('.o','.opp') and p!=player/'build_host/main.o']
    subprocess.run(['objcopy','--redefine-sym','main=sonix_application_main',str(player/'build_host/main.o'),str(work/'app_main.o')],check=True)
    libs=subprocess.check_output(['pkg-config','--libs','sdl2','freetype2','opusfile','wavpack'],text=True).split()
    subprocess.run(['gcc','-DHOST_BUILD=1','-I'+str(player),'-c',str(player/'tests/ui_fixture.c'),'-o',str(work/'fixture.o')],check=True)
    subprocess.run(['g++','-o',str(work/'fixture'),str(work/'fixture.o'),*[str(p) for p in objects],str(work/'app_main.o'),*libs,'-lpthread','-lm','-ldl','-lasound'],check=True)
    subprocess.run([str(work/'fixture'),str(card)],cwd=player,check=True,timeout=150,stdout=(work/'fixture.log').open('w'),stderr=subprocess.STDOUT)
    return card

if __name__=='__main__': create(Path(sys.argv[1]).resolve(),Path(sys.argv[2]).resolve())
