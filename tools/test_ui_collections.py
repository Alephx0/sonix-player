#!/usr/bin/env python3
"""Real LVGL/SDL regression journey. Run under WSL after building the host.

Writes candidate screenshots + live state + metrics. Optional --baseline DIR
compares normalized captures; baseline images are never created or overwritten.
"""
import argparse
import hashlib
from contextlib import closing
import json
import os
from pathlib import Path
import subprocess
import sqlite3
import shutil
import signal
import sys
import tempfile
import time
from PIL import Image, ImageChops, ImageDraw
from make_ui_fixture import create

class Session:
    def __init__(self, repo, out, baseline=None):
        self.repo,self.out,self.baseline=repo,out,baseline
        self.report={'actions':[], 'checks':[], 'screenshots':[], 'metrics':{}, 'baseline':str(baseline) if baseline else 'No approved baseline supplied'}
        self.probe=out/'probe';self.probe.mkdir()
        self.env=dict(os.environ,SONIX_UI_PROBE_DIR=str(self.probe))
        # The probe reports layout pixels; clicks are panel pixels. They differ
        # by the display size (1.25 at Large).
        self.zoom=1.0
    def command(self,*args):
        subprocess.run([sys.executable,str(self.repo/'tools/offline_ui_session.py'),str(self.repo),*map(str,args)],env=self.env,check=True,stdout=subprocess.DEVNULL)
        self.report['actions'].append(list(map(str,args)))
    def snapshot(self,command='dump'):
        response=self.probe/'response.json';response.unlink(missing_ok=True)
        request=self.probe/'pending';request.write_text(command+'\n');request.replace(self.probe/'request')
        until=time.monotonic()+5
        while not response.exists():
            if time.monotonic()>until:raise AssertionError('Native UI probe timed out')
            time.sleep(.03)
        state=json.loads(response.read_text())
        assert state['request']==command
        return state
    def setup(self,command):
        self.snapshot(command);time.sleep(1.2);return self.snapshot()
    def check(self,condition,message):
        assert condition,message
        self.report['checks'].append(message)
    def visible(self,state):
        return [n for n in state['nodes'] if not n['hidden'] and n['rect'][2]>0 and n['rect'][0]<480 and n['rect'][3]>0 and n['rect'][1]<800]
    def settled(self,limit=8):
        # Snapshots until two in a row agree: a flung list keeps coasting, and a
        # press on a moving list only stops it.
        last=None
        for _ in range(int(limit/.4)):
            time.sleep(.4);st=self.snapshot();rects=[n['rect'] for n in st['nodes'] if not n['hidden']]
            if rects==last:return st
            last=rects
        return st
    def label(self,state,text):
        return next(n for n in self.visible(state) if n.get('text')==text)
    def click_label(self,text):
        state=self.snapshot();matches=[n for n in self.visible(state) if n.get('text')==text]
        node=matches[-1] if state['modal'] else matches[0];self.click_node(node)
    def click_node(self,node):
        x,y,r,b=node['rect'];self.command('click',round((x+r)/2*self.zoom),round((y+b)/2*self.zoom))
    def pointer(self,*args):
        state=json.loads((self.repo/'artifacts/offline-ui/session.json').read_text())
        env=dict(self.env,DISPLAY=state['display'],XAUTHORITY=state['auth'])
        subprocess.run(['xdotool',*map(str,args)],env=env,check=True)
        self.report['actions'].append(['pointer',*map(str,args)])
    def capture(self,name):
        self.command('shot',name)
        source=self.repo/'artifacts/offline-ui'/f'{name}.png'
        img=Image.open(source).convert('RGB');self.check(img.size==(480,800),name+': native 480x800 render')
        if name in {'playlist-empty','liked-songs'}:
            # The target's RGB565 renderer quantizes #121212 to approximately #101010.
            self.check(all(abs(c-18)<=4 for c in img.getpixel((10,600))),name+': empty collection uses the shared dark background')
        img.save(self.out/f'{name}.png')
        # Only nondeterministic status text is masked. Real evidence remains intact.
        ImageDraw.Draw(img).rectangle((0,0,479,43),fill='#121212')
        img.save(self.out/f'{name}.normalized.png')
        # Dynamic playback captures are evidence, not deterministic pixel fixtures.
        dynamic=name in {'playlist-playing-menu','playlist-edit','playlist-added','library-playlists','liked-songs',
                         'playlist-preview'}
        if self.baseline and not dynamic:
            expected=self.baseline/f'{name}.normalized.png'
            self.check(expected.exists(),name+': baseline exists')
            diff=ImageChops.difference(img,Image.open(expected).convert('RGB'))
            diff.save(self.out/f'{name}.diff.png')
            self.check(diff.getbbox() is None,name+': matches selected baseline')
        state=self.snapshot();(self.out/f'{name}.json').write_text(json.dumps(state,indent=2))
        self.report['screenshots'].append(name+'.png');return state
    def collection_geometry(self,state):
        images=[n['rect'] for n in self.visible(state) if n['type']=='image' and n['rect'][2]-n['rect'][0]==220 and n['rect'][3]-n['rect'][1]==220]
        self.check(len(images)==1,'one centered collection cover')
        self.check(images[0][0]+images[0][2]==480,'collection cover centered')
        play=[n['rect'] for n in self.visible(state) if n['clickable'] and n['rect'][2]-n['rect'][0]==64 and n['rect'][3]-n['rect'][1]==64]
        self.check(len(play)==1,'one collection play control')
        for title in ('Home','Search','Your Library'):self.label(state,title)
        self.check(state['route']=='collection','shared native collection destination')
        return images[0],play[0]
    def metrics(self):
        state=json.loads((self.repo/'artifacts/offline-ui/session.json').read_text());pid=state['pid']
        def cpu():
            fields=Path(f'/proc/{pid}/stat').read_text().split();return int(fields[13])+int(fields[14])
        start=cpu();t=time.monotonic();time.sleep(1);elapsed=time.monotonic()-t
        status=Path(f'/proc/{pid}/status').read_text().splitlines()
        self.report['metrics']={'host_rss_kib':int(next(x for x in status if x.startswith('VmRSS:')).split()[1]),'idle_cpu_percent_one_core':round((cpu()-start)/os.sysconf('SC_CLK_TCK')/elapsed*100,2),'sample_seconds':round(elapsed,3),'lvgl':self.snapshot()['lvgl'],'note':'SDL software rendering on host; not a measurement of R1 RAM or timing'}

def journey(s,card):
    def entries(name):
        with sqlite3.connect('file:'+str(card/'.local/library.db')+'?mode=ro',uri=True) as db:
            return [r[0] for r in db.execute('SELECT path FROM "M3U_'+name.replace('"','""')+'" ORDER BY idx')]
    album=s.setup('album '+str(card/'Music/track000.flac'))
    album=s.capture('album');ag=s.collection_geometry(album)
    playlist=s.setup('playlist Night Drive');playlist=s.capture('playlist');pg=s.collection_geometry(playlist)
    s.check(ag==pg,'album and playlist share artwork and transport geometry')
    s.label(playlist,'Playlist \u00b7 6 songs')
    s.check(not playlist['file'],'opening collections does not start playback')
    # Spotify's playlist pills under the header (iOS captures 2026-11-04).
    pills=[s.label(playlist,n) for n in ('Add','Edit','Sort')]
    s.check(len({p['rect'][1] for p in pills})==1 and pills[0]['rect'][1]>pg[1][3],'Add, Edit and Sort pills sit in one row under the header')
    s.click_label('Edit');time.sleep(.8);ed=s.capture('playlist-edit-pill')
    s.check(s.label(ed,'Night Walk')['rect'][1]<240,'Edit pill opens the playlist editor (songs at the top, ready to move)')
    s.command('drag',20,400,440,410,.1);time.sleep(.8);playlist=s.setup('playlist Night Drive')
    # Open playlist actions from the shared header, then dismiss by scrim.
    _,play=pg;cy=(play[1]+play[3])//2
    s.command('click',102,cy);menu=s.capture('playlist-menu');s.check(menu['modal'],'playlist action sheet opens')
    s.command('click',240,48);s.check(not s.snapshot()['modal'],'scrim dismisses sheet')
    # Drag from the sheet's own handle, wherever a content-sized sheet puts it.
    s.command('click',102,cy);time.sleep(.4);opened=s.snapshot()
    handle=[n for n in s.visible(opened) if n['clickable'] and n['rect'][2]-n['rect'][0]==416 and n['rect'][3]-n['rect'][1]==32]
    hy=(handle[-1]['rect'][1]+handle[-1]['rect'][3])//2 if handle else 74
    s.command('drag',240,hy,240,min(hy+220,790))
    s.check(not s.snapshot()['modal'],'handle swipe dismisses downward')
    empty=s.setup('playlist Empty Playlist');s.capture('playlist-empty');s.label(empty,"Let's start building your playlist");s.label(empty,'Add to this playlist')
    _,emptyplay=s.collection_geometry(empty)
    s.command('click',(emptyplay[0]+emptyplay[2])//2,(emptyplay[1]+emptyplay[3])//2)
    s.check(not s.snapshot()['file'],'empty playlist play cannot replace playback')
    longname='A very long playlist title for evenings beside the ocean and quiet mornings'
    long=s.setup('playlist '+longname);s.capture('playlist-long')
    title=next(n for n in s.visible(long) if n.get('text','').startswith('A very long playlist title'))
    _,longplay=s.collection_geometry(long)
    s.check(title['rect'][2]<=480 and title['rect'][3]<longplay[1],'long title fits above controls')
    large=s.setup('playlist Large Playlist');s.capture('playlist-large');s.label(large,'Playlist \u00b7 257 songs')
    s.check(large['node_count']<=playlist['node_count']+100,'large playlist retains bounded widget pool')
    s.command('drag',200,600,200,260);s.capture('playlist-scrolled')
    missing=s.setup('album '+str(card/'Music/track005.flac'));s.capture('album-missing-art');s.collection_geometry(missing)
    # Rapid native pointer interactions, long-press and drag have reusable primitives.
    for i in range(3):
        s.setup('playlist Night Drive');s.command('click',102,cy);s.check(s.snapshot()['modal'],'repeated menu open')
        s.command('click',240,48);s.check(not s.snapshot()['modal'],'repeated menu close')
    for i in range(5):
        s.pointer('mousemove',102,cy,'click','1','sleep','.05','mousemove',240,48,'click','1','sleep','.3')
    time.sleep(.5);s.check(not s.snapshot()['modal'],'rapid open/close leaves no stuck overlay')
    s.setup('playlist Night Drive')
    s.command('click',(play[0]+play[2])//2,cy);time.sleep(.5)
    playing=s.snapshot();s.check(bool(playing['file']),'collection Play starts actual audio')
    s.check(len(playing['queue'])==6,'Play establishes exactly the playlist context')
    # Return to collection without changing audio, then reopen menu.
    s.check(not playing['now_playing_open'],'collection Play stays in mini-player preview');s.setup('playlist Night Drive')
    before=s.snapshot();s.command('click',102,cy);after=s.capture('playlist-playing-menu')
    s.check(after['modal'] and after['file']==before['file'] and after['status']==before['status'] and after['seconds']>=before['seconds'],'sheet preserves active playback and position')
    s.command('click',240,48)
    # Independent DB checks verify actual edit outcomes, not just labels.
    original=entries('Night Drive')
    s.command('click',102,cy);s.click_label('Edit tracks')
    edit=s.capture('playlist-edit')
    row=s.label(edit,'Caf\u00e9 Lights');parent=edit['nodes'][row['parent']];y=(parent['rect'][1]+parent['rect'][3])//2
    s.command('drag',429,y,429,y+84,.65)
    s.check(entries('Night Drive')==[original[0],original[2],original[1],*original[3:]],'drag reorder persists exact playlist order')
    s.check(not s.snapshot()['modal'],'row dragging does not open or dismiss a sheet')
    # Add a song to the empty collection through the existing local selector.
    s.setup('playlist Empty Playlist');s.command('click',102,cy);s.click_label('Add songs')
    s.check(s.snapshot()['modal'],'nested Add songs stays in the modal flow')
    s.click_label('Caf\u00e9 Lights')
    s.check(entries('Empty Playlist')==[str(card/'Music/track001.flac')],'song selector persists playlist membership')
    s.command('click',448,80);updated=s.capture('playlist-added');s.label(updated,'Playlist \u00b7 1 song')
    s.check(updated['file']==before['file'],'adding songs does not replace playback')
    # Queue insertion and shuffle use real controls, observed independently.
    s.setup('playlist Night Drive');s.command('click',102,cy);s.click_label('Add to queue')
    queued=s.snapshot();s.check(queued['priority']==6,'playlist menu adds six manual priority entries')
    # Existing Sonix success notices intercept a tap until dismissed.
    s.command('click',240,48)
    s.command('click',43,cy);toggled=s.snapshot()
    s.check(toggled['file']==queued['file'] and toggled['queue']==queued['queue'] and toggled['shuffle']==queued['shuffle'],'Shuffle toggle does not start or change playback')
    s.command('click',(play[0]+play[2])//2,cy);shuffled=s.snapshot()
    s.check(shuffled['shuffle'] and shuffled['priority']==6,'Play applies Shuffle and preserves priority entries')
    s.check(not shuffled['now_playing_open'],'collection Shuffle stays in mini-player preview')
    s.command('click',400,770);s.click_label('Playlists');filtered=s.snapshot()
    s.check(filtered['route']=='main' and filtered['tab']==2 and filtered['library_filter']==1,'Library navigation and active filter update underlying state')
    s.capture('library-playlists')
    s.click_label('Liked Songs');liked=s.capture('liked-songs');s.collection_geometry(liked)
    s.label(liked,'Playlist \u00b7 0 songs')
    s.check(liked['file']==shuffled['file'],'Liked Songs shares collection layout without changing playback')
    s.command('click',400,770)
    s.metrics()
    s.command('stop');s.command('start',3,'--card',card,'--preserve-config')
    resumed=s.snapshot()
    s.check(resumed['tab']==2 and resumed['library_filter']==1,'Library tab and filter survive restart')
    s.check(resumed['shuffle'] and resumed['priority']==6,'shuffle and manual queue survive restart')
    s.check(entries('Empty Playlist')==[str(card/'Music/track001.flac')],'playlist membership survives restart')

def preview_local(s,card):
    s.command('click',400,770)
    # Tap actual cover rectangles, not text labels. Scroll the Library as needed.
    def portrait(title):
        for _ in range(20):
            state=s.snapshot()
            matches=[n for n in s.visible(state) if n.get('text')==title and 210<n['rect'][1]<625]
            if matches:
                row=state['nodes'][matches[0]['parent']]
                covers=[n for n in state['nodes'] if n['parent']==row['id'] and n['type']=='object' and n['rect'][2]-n['rect'][0]>=60]
                cover=covers[0];x,y,r,b=cover['rect'];s.command('click',(x+r)//2,(y+b)//2);return
            s.command('drag',230,595,230,300)
        raise AssertionError('Cannot find collection portrait: '+title)
    portrait('Liked Songs');state=s.capture('portrait-liked')
    s.check(state['route']=='collection' and not state['now_playing_open'],'Liked Songs portrait opens collection')
    s.command('click',400,770);portrait('All songs');state=s.capture('portrait-all')
    s.check(state['route']=='collection','All songs portrait opens collection')
    s.label(state,'Your Library')
    s.command('click',400,770);s.command('click',430,182)
    portrait('Liked Songs');state=s.capture('portrait-liked-grid')
    s.check(state['route']=='collection','grid artwork also opens Liked Songs')
    s.command('click',400,770);s.command('click',430,182)
    s.setup('playlist Night Drive');s.click_label('Night Walk');playing=s.capture('playlist-preview')
    s.check(playing['file'].endswith('track000.flac') and playing['status']==1,'track row starts selected song')
    s.check(not playing['now_playing_open'] and playing['route']=='collection','track tap keeps playlist and mini-player visible')
    s.command('click',170,695);s.check(s.snapshot()['now_playing_open'],'explicit mini-player tap expands Now Playing')
    expanded=s.capture('now-playing-local')
    s.command('click',440,755);queue=s.capture('queue-local')
    s.check(queue['modal'] and queue['file']==expanded['file'] and queue['status']==expanded['status'],'queue icon opens sheet without interrupting playback')
    s.command('drag',240,65,240,450)
    s.check(not s.snapshot()['modal'],'queue sheet handle dismisses downward')
    s.command('drag',240,65,240,610)
    s.metrics()

def home_play(s,card):
    s.command('click',80,770);home=s.capture('home-before-play')
    s.check(home['route']=='main' and not home['file'],'Home opens without starting playback')
    # Spotify's shelf cards carry no overlay buttons: Play is in the card's
    # long-press menu.
    cards=[n for n in s.visible(home) if n['clickable'] and n['rect'][2]-n['rect'][0]==150 and n['rect'][1]<450]
    if not cards:  # the shelves start below the shortcuts on a fresh card
        s.command('drag',240,600,240,250,.05);home=s.settled()  # the fling coasts before a press counts
        cards=[n for n in s.visible(home) if n['clickable'] and n['rect'][2]-n['rect'][0]==150 and 120<n['rect'][1]<450]
    s.check(cards,'Home shows 150 px shelf cards')
    s.check(not any(n['clickable'] and n['rect'][2]-n['rect'][0]==42 and n['rect'][3]-n['rect'][1]==42 for n in s.visible(home)),'shelf cards carry no overlay Play button')
    x,y,r,b=cards[0]['rect'];s.command('drag',(x+r)//2,y+60,(x+r)//2+1,y+60,1.2);time.sleep(.5)
    menu=s.snapshot();s.check(menu['modal'],'long press opens the card menu at %r'%(cards[0]['rect'],));s.click_label('Play collection');time.sleep(1)
    playing=s.capture('home-play-preview')
    s.check(playing['file'] and playing['status']==1,'Home Play starts its collection')
    s.check(playing['route']=='main' and not playing['now_playing_open'],'Home Play keeps Home and the mini-player visible')
    s.command('click',170,695);s.check(s.snapshot()['now_playing_open'],'explicit mini-player tap expands Now Playing')
    s.metrics()

def device_settings(s,card):
    LEGACY_BLUE=(53,132,228)
    def no_legacy_accent(name):
        img=Image.open(s.out/f'{name}.png').convert('RGB')
        hits=sum(1 for p in img.getdata() if all(abs(a-b)<=6 for a,b in zip(p,LEGACY_BLUE)))
        s.check(hits==0,name+': no legacy Adwaita accent pixels')
    def footer(state,name):
        for t in ('Home','Search','Your Library'):s.label(state,t)
        mini=[n for n in s.visible(state) if n.get('text')=='Choose something to play' or n.get('text')=='Night Walk']
        s.check(mini,name+': mini-player stays mounted')
        lists=[n for n in s.visible(state) if n.get('scrollable') and n['parent']!=-1 and n['rect'][1]>=100 and n['rect'][2]-n['rect'][0]>=400 and n['rect'][3]-n['rect'][1]>=200]
        s.check(lists and all(n['rect'][3]<=800-144 for n in lists),name+': list ends above mini-player and navigation')
    def profile():
        st=s.snapshot()
        btn=[n for n in s.visible(st) if n['clickable'] and n['rect'][2]-n['rect'][0]==44 and n['rect'][1]<140 and n['rect'][0]<60]
        s.check(len(btn)==1,'one device menu button beside the tab title')
        x,y,r,b=btn[0]['rect'];s.command('click',(x+r)//2,(y+b)//2)
    s.setup('playlist Night Drive');s.click_label('Night Walk');playing=s.snapshot()
    s.check(playing['status']==1,'fixture playback running before settings journey')
    s.command('click',80,770);home=s.capture('device-home')
    s.check(not any(n['clickable'] and n['rect'][0]>=340 and n['rect'][1]<140 and n['rect'][2]-n['rect'][0] in (48,60) for n in s.visible(home)),'legacy tools and gear buttons are not reachable')
    profile();menu=s.capture('device-menu')
    s.check(menu['modal'],'profile drawer opens over the page')
    for t in ('Settings','Devices','Audiobooks','Browse music','More'):s.label(menu,t)
    s.command('click',450,400);time.sleep(.5);s.check(not s.snapshot()['modal'],'scrim dismisses the profile drawer')
    profile();s.click_label('Settings');time.sleep(.8)
    root=s.capture('device-settings-root');no_legacy_accent('device-settings-root');footer(root,'Settings')
    s.label(root,'Settings')
    rows=[n for n in s.visible(root) if n['clickable'] and n['rect'][2]-n['rect'][0]>=400 and 100<n['rect'][1]<640 and n['rect'][3]-n['rect'][1]<=120]
    # The device's card heads the page (round 4: its name and picture shown on Settings).
    card=[n for n in rows if n['rect'][3]-n['rect'][1]==84]
    s.check(len(card)==1 and any(t.get('text')=='Sonix R1' and card[0]['rect'][1]<=t['rect'][1]<card[0]['rect'][3] for t in s.visible(root)),'the device card with its name heads Settings')
    rows=[n for n in rows if n not in card]
    s.check(rows and all(n['rect'][3]-n['rect'][1]==72 for n in rows),'Settings rows use the 72 px reference height')
    s.click_label('Content and display');time.sleep(.8)
    s.click_label('Screen');time.sleep(.8);screen=s.capture('device-settings-screen')
    no_legacy_accent('device-settings-screen');footer(screen,'Screen')
    sw=[n for n in s.visible(screen) if n['type']=='switch']
    s.check(sw and all(n['rect'][2]-n['rect'][0]==52 and n['rect'][3]-n['rect'][1]==32 for n in sw),'switches use the 52 x 32 reference size')
    x,y,r,b=sw[0]['rect'];before=sw[0].get('checked');s.command('click',(x+r)//2,(y+b)//2)
    after=next(n for n in s.snapshot()['nodes'] if n['id']==sw[0]['id']).get('checked')
    s.check(before!=after,'tapping a switch toggles its real state')
    s.command('click',(x+r)//2,(y+b)//2)
    s.command('click',400,770);lib=s.snapshot()
    s.check(lib['route']=='main' and lib['tab']==2,'navigation from a settings page returns to Your Library')
    s.check(lib['status']==1 and lib['file']==playing['file'],'settings journey never interrupts playback')
    s.command('click',80,770);profile();s.click_label('Devices');time.sleep(.8)
    dev=s.capture('device-hub-wireless');no_legacy_accent('device-hub-wireless');footer(dev,'Devices hub')
    for t in ('Wi-Fi','Bluetooth'):s.label(dev,t)
    s.check(not any(n['type']=='image' and n['rect'][2]-n['rect'][0]>=90 for n in s.visible(dev)),'hub lists rows, not legacy picture tiles')
    # Result feedback is a non-blocking snackbar above the footer.
    s.setup('playlist Night Drive');before=s.snapshot()['priority']
    s.command('drag',240,560,240,300,.3);s.settled()  # past the header's pills
    row=s.label(s.snapshot(),'Café Lights');y=(row['rect'][1]+row['rect'][3])//2
    s.command('drag',120,y,330,y+4,.1);snack=s.capture('snackbar-queue')
    s.check(snack['priority']==before+1,'row swipe adds the track to the priority queue')
    bar=[n for n in s.visible(snack) if n.get('text') and 'queue' in n['text'].lower() and n['rect'][1]>400]
    s.check(bar and bar[0]['rect'][3]<=800-144,'queue feedback appears as a snackbar above the mini-player')
    no_legacy_accent('snackbar-queue')
    s.command('click',400,770);s.check(s.snapshot()['route']=='main','snackbar does not block taps on the page')
    # Confirmation uses the reference dialog; Cancel changes nothing.
    s.command('click',80,770);profile();s.click_label('Audiobooks');time.sleep(.8)
    hub=s.snapshot();icons=[n for n in s.visible(hub) if n['clickable'] and n['rect'][1]<110 and n['rect'][0]>380]
    x,y,r,b=max(icons,key=lambda n:n['rect'][0])['rect'];s.command('click',(x+r)//2,(y+b)//2);time.sleep(.8)
    opts=s.snapshot();scan=[n for n in s.visible(opts) if n.get('text','').lower().startswith('scan')]
    s.check(scan,'audiobook options offer a scan action')
    x,y,r,b=scan[0]['rect'];s.command('click',(x+r)//2,(y+b)//2);dialog=s.capture('dialog-scan-confirm')
    no_legacy_accent('dialog-scan-confirm')
    cancel=s.label(dialog,'Cancel');s.check(cancel,'confirmation offers Cancel')
    x,y,r,b=cancel['rect'];s.command('click',(x+r)//2,(y+b)//2);after=s.snapshot()
    s.check(after['route']=='other' and not any(n.get('text')=='Cancel' for n in s.visible(after)),'Cancel closes the dialog without starting a scan')
    # The audiobook library list uses flat reference rows with the footer.
    s.command('drag',20,400,440,410,.1);time.sleep(.8);s.click_label('Library');time.sleep(.8)
    books=s.capture('audiobook-library-list');no_legacy_accent('audiobook-library-list');footer(books,'Audiobook library')
    rows=[n for n in s.visible(books) if n['clickable'] and n['rect'][2]-n['rect'][0]>=400 and 100<n['rect'][1]<640 and n['rect'][3]-n['rect'][1]<=120]
    s.check(len(rows)>=3 and all(n['rect'][3]-n['rect'][1]==76 for n in rows),'audiobook rows use the 76 px reference height')
    s.check(s.snapshot()['status']==1,'audiobook browsing keeps music playing')
    # Option menus (former anchored popovers) open in the shared sheet.
    corner=[n for n in s.visible(books) if n['clickable'] and n['rect'][1]<110 and n['rect'][0]>380]
    x,y,r,b=max(corner,key=lambda n:n['rect'][0])['rect'];s.command('click',(x+r)//2,(y+b)//2)
    menu=s.capture('option-menu-sheet');no_legacy_accent('option-menu-sheet')
    s.check(menu['modal'],'sort options open in the shared bottom sheet')
    choices=[n for n in s.visible(menu) if n['clickable'] and n['rect'][1]>400 and n['rect'][3]-n['rect'][1]==60]
    s.check(len(choices)>=2,'sheet lists each option as a 60 px row')
    s.check(all(c['rect'][3]<=792 for c in choices),'last option clears the bottom edge')
    img=Image.open(s.out/'option-menu-sheet.png').convert('RGB')
    s.check(any(all(abs(a-b)<=8 for a,b in zip(img.getpixel((px,py)),(30,215,96))) for c in choices for px in range(c['rect'][2]-50,c['rect'][2]-6,2) for py in range(c['rect'][1]+20,c['rect'][3]-20,2)),'current option carries the green check')
    x,y,r,b=choices[-1]['rect'];s.command('click',(x+r)//2,(y+b)//2)
    s.check(not s.snapshot()['modal'],'choosing an option closes the sheet')
    s.metrics()

def player_core(s,card):
    s.setup('playlist Night Drive');s.click_label('Night Walk');time.sleep(.5)
    s.command('click',170,695);np=s.snapshot()
    s.check(np['now_playing_open'] and np['status']==1,'mini-player expands Now Playing while playing')
    rep0=np['repeat'];s.command('click',437,683);want=s.snapshot()['repeat']
    s.check(want!=rep0,'Now Playing repeat button changes the repeat mode')
    full=s.capture('player-now-playing')
    s.check(not any(n.get('text','').startswith('PLAYING FROM') for n in s.visible(full)),'the header names only the context, as Spotify 2026 does')
    s.check(full['context_kind']=='PLAYLIST','playlist playback records its context kind')
    s.check(any(n.get('text')=='Night Drive' and n['rect'][1]<100 for n in s.visible(full)),'Now Playing header names the playing context')
    dots=[n for n in s.visible(full) if n['rect'][2]-n['rect'][0]==5 and n['rect'][3]-n['rect'][1]==5]
    s.check(len(dots)==1,'active repeat shows one green mode dot')
    top=lambda txt:max((n for n in s.visible(full) if n.get('text')==txt),key=lambda n:n['id'])
    artist=top('Night Ensemble');bar=[n for n in s.visible(full) if n['type']=='slider' and n['rect'][2]-n['rect'][0]>300]
    title=top('Night Walk')
    s.check(artist['rect'][1]>=title['rect'][3]-2,'title and artist lines do not overlap')
    s.check(bar and bar[0]['rect'][1]-artist['rect'][3]>=8,'artist line is clear of the progress bar')
    # Queue sheet header: title at the sheet's text edge, corner buttons clear of close.
    import re
    np=s.snapshot();s.check(any(re.fullmatch(r'-\d+:\d\d',n.get('text','')) for n in s.visible(np)),'Now Playing counts down the time left ("-2:57"), as Spotify')
    s.command('click',440,755);q=s.capture('player-queue-sheet')
    s.check(q['modal'],'queue opens in the shared sheet')
    title=s.label(q,'Queue');s.check(title['rect'][0]<=40,'queue title starts at the sheet text edge')
    # Only the sheet's own nodes: Now Playing's buttons lie beneath its scrim.
    scrim=max(n['id'] for n in q['nodes'] if n['rect']==[0,0,480,800])
    small=[n for n in s.visible(q) if n['id']>scrim and n['clickable'] and n['rect'][3]<140 and n['rect'][2]-n['rect'][0]<=64 and n['rect'][3]-n['rect'][1]<=64]
    close=max(small,key=lambda n:n['rect'][2])
    others=[n for n in small if n is not close and n['rect'][0]>240]
    s.check(all(n['rect'][2]<=close['rect'][0] for n in others),'queue corner buttons do not overlap the sheet close button')
    # Spotify's queue sheet (iOS captures 2026-11-04): Edit in the corner,
    # Shuffle / Repeat / Timer tiles along the bottom; Edit Queue swaps the
    # tiles for Move up and Remove and gives each row a round selector.
    for t in ('Edit','Shuffle','Repeat','Timer'):s.label(q,t)
    s.check(all(s.label(q,t)['rect'][1]>660 for t in ('Shuffle','Repeat','Timer')),'mode tiles sit along the bottom of the sheet')
    s.check(s.label(q,'Edit')['rect'][2]<=close['rect'][0],'Edit pill stays clear of the sheet close button')
    rep0=q['repeat'];s.click_label('Repeat');time.sleep(.3)
    s.check(s.snapshot()['repeat']==(rep0+1)%3,'Repeat tile steps the repeat mode')
    s.click_label('Repeat');s.click_label('Repeat');time.sleep(.3);s.check(s.snapshot()['repeat']==rep0,'Repeat tile cycles back')
    s.click_label('Edit');time.sleep(.5);ed=s.capture('player-queue-edit')
    for t in ('Edit Queue','Move up','Remove'):s.label(ed,t)
    s.check(not any(n.get('text') in ('Shuffle','Timer') for n in s.visible(ed)),'Edit Queue replaces the tiles')
    rings=[n for n in s.visible(ed) if n['type']=='object' and n['rect'][2]-n['rect'][0]==26 and n['rect'][3]-n['rect'][1]==26]
    s.check(len(rings)>=1,'rows that can be chosen carry a round selector (%d)'%len(rings))
    x,y,r,b=[n for n in s.visible(ed) if n['clickable'] and n['rect'][3]<140 and n['rect'][2]-n['rect'][0]<=64 and n['id']>scrim and n['rect'][0]>240 and n['rect'][2]<close['rect'][0]+1][-1]['rect']
    s.command('click',(x+r)//2,(y+b)//2);time.sleep(.4);s.label(s.snapshot(),'Edit')
    s.command('drag',240,65,240,450);s.check(not s.snapshot()['modal'],'queue sheet handle dismisses downward')
    # Track information opens a visible page and Back returns to the player.
    s.command('click',436,56);time.sleep(.5);s.command('drag',240,600,240,250,.3);time.sleep(.6);s.click_label('Track information / File details');time.sleep(.8)
    det=s.capture('player-details')
    s.check(not det['now_playing_open'] and any(n.get('text')=='Details' for n in s.visible(det)),'Track information shows the details page, not hidden under the player')
    s.check(det['status']==1,'opening details keeps playback')
    s.command('drag',20,400,440,410,.1);time.sleep(.8)
    s.check(s.snapshot()['now_playing_open'],'Back from details returns to Now Playing')
    # Go to radio: the song, then similar songs from the library, named as Spotify names a radio.
    s.command('click',436,56);time.sleep(.5);s.click_label('Go to radio');time.sleep(1);radio=s.snapshot()
    s.check(radio['context_kind']=='RADIO' and radio['context_name'].endswith(' Radio'),'Go to radio plays a radio of the song (%s)'%radio['context_name'])
    s.check(radio['status']==1 and len(radio['queue'])>1,'the radio continues with other library songs')
    # Snooze (Spotify, Sep 2025): out of Autoplay and radio for 30 days, and back.
    snoozed=lambda f:sqlite3.connect(str(card/'.local/library.db')).execute('SELECT COUNT(*) FROM SNOOZE WHERE path=?',(f,)).fetchone()[0]
    s.command('click',436,56);time.sleep(.6);s.click_label('Snooze for 30 days');time.sleep(.5)
    s.check(snoozed(radio['file'])==1,'Snooze keeps the playing song out for 30 days')
    s.command('click',436,56);time.sleep(.6);s.click_label('Unsnooze');time.sleep(.5)
    s.check(snoozed(radio['file'])==0,'Unsnooze lifts it again')
    # R1 additions (not in Spotify): the decoded format as a badge, a Sound
    # sheet of the DSP switches, and saving the queue as a playlist.
    np=s.capture('player-r1-row');vis=s.visible(np)
    s.check(any(re.fullmatch(r'\d+/[\d.]+ FLAC',n.get('text','')) for n in vis),'Now Playing shows the format badge (16/44.1 FLAC)')
    snd=[n for n in vis if n['clickable'] and 40<=n['rect'][2]-n['rect'][0]<=56 and 720<n['rect'][1]<760 and 330<n['rect'][0]<400]
    s.check(len(snd)==1,'a Sound button sits beside the queue button %r'%[(n['type'],n['clickable'],n['rect']) for n in vis if 620<n['rect'][1]<710 and n['rect'][0]>300])
    x,y,r,b=snd[0]['rect'];s.command('click',(x+r)//2,(y+b)//2);time.sleep(.8);sheet=s.capture('player-sound-sheet')
    for t in ('Sound','Equalizer','MSEB','Crossfeed','Volume normalization','Off','Track','Album'):s.label(sheet,t)
    s.check(sheet['modal'],'Sound opens as a sheet over Now Playing')
    s.command('click',240,90);time.sleep(.6);s.check(not s.snapshot()['modal'],'tapping above the sheet closes it')
    # Sound per output (R1): wired and Bluetooth keep their own switches.
    def sound(open_only=False):
        x,y,r,b=snd[0]['rect'];s.command('click',(x+r)//2,(y+b)//2);time.sleep(.8);sh=s.snapshot()
        lab=s.label(sh,'MSEB');sw=[n for n in s.visible(sh) if n['type']=='switch' and abs((n['rect'][1]+n['rect'][3])/2-(lab['rect'][1]+lab['rect'][3])/2)<30][0]
        return sh,sw
    def tap(n):x,y,r,b=n['rect'];s.command('click',(x+r)//2,(y+b)//2);time.sleep(.4)
    sh,sw=sound();s.check(any('Kept for Wired output' in n.get('text','') for n in s.visible(sh)),'the Sound sheet says which output it is kept for')
    if not sw.get('checked'):tap(sw)
    s.command('click',240,90);time.sleep(.5);s.snapshot('output bt');time.sleep(1.6)
    sh,sw=sound();s.check(any('Kept for Bluetooth output' in n.get('text','') for n in s.visible(sh)),'Bluetooth output is recognised')
    s.check(sw.get('checked'),'a new output starts from the sound in use');tap(sw)
    s.command('click',240,90);time.sleep(.5);s.snapshot('output wired');time.sleep(1.6)
    sh,sw=sound();s.check(sw.get('checked'),'back on wired, its own MSEB setting returns')
    s.command('click',240,90);time.sleep(.5);s.snapshot('output bt');time.sleep(1.6)
    sh,sw=sound();s.check(not sw.get('checked'),'and on Bluetooth, the Bluetooth one')
    s.command('click',240,90);time.sleep(.5);s.snapshot('output auto');time.sleep(1.2)
    s.command('click',436,56);time.sleep(.8);s.capture('player-menu-r1');s.click_label('Save queue as playlist');time.sleep(.6)
    with sqlite3.connect(str(card/'.local/library.db')) as db:
        names=[r[0] for r in db.execute("SELECT name FROM sqlite_master WHERE name LIKE 'M3U_Queue %'")]
        rows=db.execute('SELECT COUNT(*) FROM "%s"'%names[0]).fetchone()[0] if names else 0
    s.check(names and rows==len(radio['queue'])-radio['queue'].index(radio['file']),'Save queue as playlist stores the playing song and what follows (%r, %d rows)'%(names,rows))
    s.command('drag',240,65,240,610);s.check(not s.snapshot()['now_playing_open'],'Now Playing collapses downward')
    # Collection Shuffle then Play keeps the repeat the user chose.
    col=s.setup('playlist Night Drive');_,play=s.collection_geometry(col);cy=(play[1]+play[3])//2
    s.command('click',42,cy);s.command('click',(play[0]+play[2])//2,cy);time.sleep(.6);st=s.snapshot()
    s.check(st['shuffle'] and st['repeat']==want,'Shuffle + Play applies shuffle without resetting repeat')
    s.check(not st['now_playing_open'],'collection Play stays in preview')
    s.check(st['queue'] and st['queue'][0]==st['file'] and len(st['queue'])==6,'shuffled Play starts a fresh deal with every other song still to come')
    def context_label():
        s.command('click',170,695);time.sleep(.6)
        s.check(s.snapshot()['now_playing_open'],'mini-player expands Now Playing for the queue')
        s.command('click',440,755);time.sleep(.6);qs=s.capture('player-queue-context')
        s.check(qs['modal'],'queue sheet open for the context check')
        names=[n['text'] for n in s.visible(qs) if n.get('text','').startswith('Next from')]
        s.command('drag',240,65,240,450);s.command('drag',240,65,240,610);return names
    names=context_label();s.check(names==['Next from Night Drive'],'queue names its context "Next from Night Drive": '+repr(names))
    dealt=s.snapshot()
    s.command('stop');s.command('start',3,'--card',card,'--preserve-config');time.sleep(1)
    back=s.snapshot()
    s.check(back['queue']==dealt['queue'] and back['file']==dealt['file'],'restart restores the fresh shuffle deal and its position')
    s.check(context_label()==['Next from Night Drive'],'context name survives restart with the restored queue')
    s.check(s.snapshot()['context_kind']=='PLAYLIST','context kind survives restart with the restored queue')
    # Track menu: Add to queue uses the manual queue; Add to playlist uses the membership sheet.
    def row_menu(title):
        st=s.snapshot();lab=s.label(st,title);y=(lab['rect'][1]+lab['rect'][3])//2
        btn=[n for n in s.visible(st) if n['clickable'] and n['rect'][0]>380 and n['rect'][1]<=y<=n['rect'][3] and n['rect'][2]-n['rect'][0]<=80]
        x,y0,r,b=btn[0]['rect'];s.command('click',(x+r)//2,(y0+b)//2)
    s.setup('album '+str(card/'Music/track000.flac'));base=s.snapshot();before=base['priority']
    row_menu('Night Walk');s.click_label('Add to queue');time.sleep(.4)
    s.check(s.snapshot()['priority']==before+1,'track menu Add to queue appends to the manual queue')
    row_menu('Night Walk');s.click_label('Add to playlist');time.sleep(.6)
    member=s.capture('player-add-to-playlist')
    s.check(member['modal'] and member['route']=='collection','Add to playlist opens the membership sheet over the collection')
    s.label(member,'New playlist');s.check(any(n['type']=='textarea' for n in s.visible(member)),'membership sheet offers playlist search')
    s.command('click',240,30);s.check(member['status']==base['status'] and member['file']==base['file'],'playlist actions leave playback state unchanged')
    # The album header's artist text opens the credited artist's discography.
    alb=s.setup('album '+str(card/'Music/track000.flac'))
    head=[n for n in s.visible(alb) if n.get('text')=='Night Ensemble' and n['rect'][1]<460]
    s.check(head and head[0]['clickable'],'album header artist text is tappable')
    x,y,r,b=head[0]['rect'];s.command('click',(x+r)//2,(y+b)//2);time.sleep(.8);art=s.snapshot()
    if art['modal']:
        s.click_label('Night Ensemble');time.sleep(.8);art=s.snapshot()
    s.check(any(n.get('text')=='Featured On' for n in s.visible(art)),'artist text reaches the discography page')
    s.check(not any(n.get('text') in ('More releases','Previous releases','See all') for n in art['nodes'] if not n['hidden']),'the catalogue scrolls whole, with no More button')
    s.capture('player-artist-discography')
    # Selection mode's Add to playlist uses the shared sheet, listing destinations.
    def song_count():
        st=s.setup('playlist Night Drive')
        meta=[n['text'] for n in s.visible(st) if n.get('text','').startswith('Playlist ·')]
        return int(meta[0].split('·')[1].split()[0]) if meta else -1
    before=song_count()
    alb=s.setup('album '+str(card/'Music/track000.flac'));row=s.label(alb,'Night Walk');y=(row['rect'][1]+row['rect'][3])//2
    s.command('drag',200,y,201,y,1.4);time.sleep(.5);sel=s.snapshot()
    corner=sorted([n for n in s.visible(sel) if n['clickable'] and n['rect'][1]<140 and n['rect'][0]>150 and n['rect'][2]-n['rect'][0]<=64 and n['rect'][3]-n['rect'][1]<=64],key=lambda n:n['rect'][0])
    s.check(len(corner)==4,'long press enters selection with four corner actions: %d'%len(corner))
    x,y0,r,b=corner[2]['rect'];s.command('click',(x+r)//2,(y0+b)//2);time.sleep(.6)
    bulk=s.capture('selection-add-to-playlist')
    s.check(bulk['modal'] and bulk['route']=='collection','selection Add to playlist opens the shared sheet, not the legacy picker')
    s.label(bulk,'Add to playlist');s.label(bulk,'1 song');s.label(bulk,'New playlist');s.label(bulk,'Liked Songs')
    s.click_label('Night Drive');time.sleep(2)
    s.check(not s.snapshot()['modal'],'choosing a destination closes the sheet')
    s.check(song_count()==before+1,'the chosen song is added to the destination playlist')
    # Long-track resume (R1): a long track left partway starts where it was left.
    # The threshold is lowered to a minute so the 3-minute fixture qualifies.
    s.command('stop');ini=s.repo/'artifacts/offline-ui/device_config.ini'
    ini.write_text(ini.read_text().replace('[player]\n','[player]\nlong_resume_secs=60\nshuffle_albums=1\n',1))
    s.command('start',3,'--card',card,'--preserve-config');time.sleep(1)
    s.setup('playlist Night Drive');s.command('drag',240,520,240,300,.3);s.settled()
    s.click_label('Night Walk');time.sleep(1);s.snapshot('seek 90');time.sleep(1)
    s.click_label('Café Lights');time.sleep(1.5);s.click_label('Night Walk');time.sleep(1.5);back=s.snapshot()
    s.check(back['file'].endswith('track000.flac') and 85<=back['seconds']<=100,'a long track resumes where it was left (%.1f s)'%back['seconds'])
    s.snapshot('seek 175');time.sleep(.5);s.click_label('Café Lights');time.sleep(1.5);s.click_label('Night Walk');time.sleep(1.5)
    s.check(s.snapshot()['seconds']<20,'a long track finished starts from the top next time')
    # Lyrics timing (R1): a per-song delay, kept across a restart.
    s.command('click',170,695);time.sleep(.8);s.command('click',436,56);time.sleep(.6);s.click_label('Lyrics timing');time.sleep(.6)
    lt=s.capture('lyrics-timing');s.label(lt,'Lyrics timing');s.label(lt,'In sync with the file')
    s.click_label('Later');s.click_label('Later');time.sleep(.3);s.label(s.snapshot(),'Lyrics later by 0.50 s')
    s.command('stop');s.command('start',3,'--card',card,'--preserve-config');time.sleep(1.5)
    s.command('click',170,695);time.sleep(.8);s.command('click',436,56);time.sleep(.6);s.click_label('Lyrics timing');time.sleep(.6)
    s.label(s.snapshot(),'Lyrics later by 0.50 s');s.click_label('Reset');time.sleep(.3);s.label(s.snapshot(),'In sync with the file')
    s.check(True,'lyrics timing is kept per song across a restart and resets')
    # Album shuffle (R1, on through the config above): a shuffled list keeps each album together.
    col=s.setup('playlist Night Drive');_,play=s.collection_geometry(col);cy=(play[1]+play[3])//2
    if not col['shuffle']:s.command('click',42,cy)
    s.command('click',(play[0]+play[2])//2,cy);time.sleep(1);q=s.snapshot()['queue']
    with sqlite3.connect(str(card/'.local/library.db')) as db:
        alb=[ (db.execute('SELECT album FROM MEDIA_TABLE WHERE path=?',(p,)).fetchone() or [''])[0] for p in q]
    runs=[a for i,a in enumerate(alb) if i==0 or alb[i-1]!=a]
    s.check(len(runs)==len(set(runs)),'album shuffle plays each album in one run (%r)'%alb)
    # Hide in this playlist (Spotify, Sep 2025): the song stays, playback goes past it.
    s.snapshot('end-track');time.sleep(.5)
    if s.snapshot()['now_playing_open']:s.command('drag',240,65,240,610);time.sleep(.8)
    col=s.setup('playlist Night Drive')
    if col['shuffle']:
        _,play=s.collection_geometry(col);s.command('click',42,(play[1]+play[3])//2);time.sleep(.4)  # in order, so the next song is known
    for _ in range(4):
        if any(n.get('text')=='Café Lights' and n['rect'][3]<600 for n in s.visible(s.snapshot())):break
        s.command('drag',240,520,240,330,.3);s.settled()
    row_menu('Café Lights');time.sleep(.5);s.click_label('Hide in this playlist');time.sleep(.5)
    with sqlite3.connect(str(card/'.local/library.db')) as db:hidden=db.execute("SELECT COUNT(*) FROM PLAYLIST_HIDE WHERE playlist='Night Drive'").fetchone()[0]
    s.check(hidden==1,'the song is hidden in this playlist only')
    s.click_label('Night Walk');time.sleep(1);s.snapshot('end-track');time.sleep(1);nxt=s.snapshot()
    s.check(nxt['context_name']=='Night Drive' and not nxt['file'].endswith('track001.flac'),'playing the playlist goes past the hidden song (%s)'%nxt['file'])
    s.check(any(n.get('text')=='Café Lights' for n in s.visible(s.snapshot())),'the hidden song stays in the playlist')
    # Spotify's Up next and Credits cards, as a sheet from View song credits.
    if s.snapshot()['now_playing_open']:s.command('drag',240,65,240,610);time.sleep(.8)
    s.setup('playlist Night Drive');s.click_label('Night Walk');time.sleep(1);s.command('click',170,695);time.sleep(.8)
    s.command('click',436,56);time.sleep(.6);s.command('drag',240,600,240,250,.3);time.sleep(.6);s.click_label('View song credits');time.sleep(.8)
    cr=s.capture('song-credits')
    for t in ('Up next','Open Queue','Credits','Night Ensemble','Main Artist','Composer','Clara Weiss'):s.label(cr,t)
    s.command('click',240,60);time.sleep(.5);s.command('drag',240,65,240,610);time.sleep(.6)
    # Composers (R1): Browse all lists who wrote the music; one opens their tracks.
    s.command('click',240,770);time.sleep(1)
    for _ in range(4):
        if any(n.get('text')=='Composers' and n['rect'][3]<640 for n in s.visible(s.snapshot())):break
        s.command('drag',240,560,240,260,.3);s.settled()
    s.click_label('Composers');time.sleep(1);cp=s.capture('composers');s.label(cp,'Clara Weiss');s.label(cp,'2 tracks')
    s.click_label('Clara Weiss');time.sleep(1);ct=s.snapshot()
    s.check(any(n.get('text')=='Night Walk' for n in s.visible(ct)) and any(n.get('text')=='First Light' for n in s.visible(ct)),'a composer opens the tracks they wrote')
    # Smart Shuffle (Spotify): the third step of the shuffle control mixes picks in.
    if s.snapshot()['now_playing_open']:s.command('drag',240,65,240,610);time.sleep(.8)
    s.setup('playlist Night Drive');s.click_label('Night Walk');time.sleep(1);s.command('click',170,695);time.sleep(.8)
    s.command('click',440,755);time.sleep(.8)
    with sqlite3.connect(str(card/'.local/library.db')) as db:members={r[0] for r in db.execute('SELECT path FROM "M3U_Night Drive"')}
    for _ in range(3):
        if any(n.get('text')=='Smart Shuffle' for n in s.visible(s.snapshot())):break
        s.click_label('Shuffle');time.sleep(.8)
    sm=s.capture('smart-shuffle');s.label(sm,'Smart Shuffle')
    picks=[p for p in sm['queue'] if p not in members]
    s.check(picks,'Smart Shuffle mixes library songs into the queue (%d)'%len(picks))
    s.check(any(n['type']=='image' and n['rect'][2]-n['rect'][0]==24 and not n['hidden'] for n in s.visible(sm)),'the picks are marked in the queue')
    s.click_label('Smart Shuffle');time.sleep(.8);off=s.snapshot()
    s.check(not [p for p in off['queue'] if p not in members] and not off['shuffle'],'turning it off takes the picks out and shuffle off')
    s.command('drag',240,65,240,450);time.sleep(.6)
    s.metrics()

def local_search(s,card):
    def result(title):
        node=next(n for n in s.visible(s.snapshot()) if n.get('text')==title and 250<n['rect'][1]<650)
        x,y,r,b=node['rect'];s.command('click',(x+r)//2,(y+b)//2)
    s.setup('playlist Night Drive');s.click_label('Night Walk');before=s.snapshot()
    s.command('click',240,770)
    browse=s.capture('local-search-browse')
    texts=[n.get('text','') for n in s.visible(browse)]
    s.check(not any(t in texts for t in ('Sources','Online','Downloads','SpotBye')),'Search exposes only local music')
    # The Playlists tile lands in Your Library on the Playlists chip, not a legacy page.
    # The tile sits in Browse all, below the genre carousel: bring it above the mini-player.
    tile=s.label(browse,'Playlists')
    if tile['rect'][3]>640:
        s.command('drag',30,600,30,300,.05);browse=s.settled();tile=s.label(browse,'Playlists')
    x,y,r,b=tile['rect'];s.command('click',(x+r)//2,(y+b)//2);time.sleep(.6);pl=s.snapshot()
    s.check(pl['route']=='main' and pl['tab']==2 and pl['library_filter']==1,'Search Playlists tile opens Your Library filtered to playlists')
    s.click_label('Playlists');time.sleep(.4)  # chip off again
    s.check(s.snapshot()['library_filter']==0,'the Playlists chip toggles back to All')
    s.command('click',240,770);time.sleep(.4)
    s.setup('search-text night');time.sleep(1)
    results=s.capture('local-search-all');s.label(results,'Night Walk')
    s.check(results['file']==before['file'] and results['status']==1,'search preserves audio')
    s.click_label('Songs');songs=s.capture('local-search-songs')
    s.check(not any(n.get('text')=='Night Ensemble' and n['rect'][1]<650 for n in s.visible(songs)),'Songs filter excludes artist collection rows')
    result('Night Walk');playing=s.snapshot()
    s.check(playing['file'].endswith('track000.flac') and not playing['now_playing_open'],'search track starts in mini-player preview')
    s.check(playing['context_album']!='' and len(playing['queue'])>=1,'searched song plays in its album context, not a folder queue')
    music=sorted(str(x) for x in (card/'Music').iterdir() if x.suffix in ('.flac','.mp3'))
    s.check(len(playing['queue'])<len(music),'search playback does not queue the whole folder')
    s.click_label('Artists');artists=s.capture('local-search-artists')
    s.check(not any(';' in n.get('text','') and 250<n['rect'][1]<650 for n in s.visible(artists)),'artist search exposes individual credits')
    result('Night Ensemble');disc=s.capture('local-artist-discography')
    s.check(disc['file']==playing['file'],'artist browsing preserves playback')
    s.command('click',240,770);s.setup('search-text cafE')
    s.click_label('Songs');matches=s.capture('local-search-accent');s.label(matches,'Caf\u00e9 Lights')
    # A full group continues in the shared collection list of every match.
    s.click_label('Songs');s.setup('search-text without');time.sleep(1);many=s.capture('local-search-see-all')
    s.check(not any(n.get('text')=='See all results' for n in s.visible(many)),'no route to the legacy search page')
    st=many;seeall=[n for n in st['nodes'] if n.get('text')=='See all songs' and not n['hidden']]
    s.check(len(seeall)==1,'a full Songs group offers "See all songs"')
    s.check(not any(n.get('text') in ('See all albums','See all artists') for n in st['nodes'] if not n['hidden']),'groups below the instant limit offer no See all')
    # A chip narrows to a longer list of one kind (Spotify search filters).
    s.click_label('Songs');time.sleep(1);chip=s.capture('local-search-songs-chip')
    songs=[n for n in chip['nodes'] if not n['hidden'] and n.get('text')=='Without a Picture']
    s.check(len(songs)==30,'Songs chip lists up to 30 matching songs, not the 6 instant rows: %d'%len(songs))
    s.check(not any(n.get('text') in ('Albums','Artists') and n['rect'][1]>240 for n in chip['nodes'] if not n['hidden']),'Songs chip shows songs only')
    s.check(any(n.get('text')=='See all songs' for n in chip['nodes'] if not n['hidden']),'a full chip list still continues in See all songs')
    s.click_label('Songs');time.sleep(1);st=s.snapshot()
    seeall=[n for n in st['nodes'] if n.get('text')=='See all songs' and not n['hidden']]
    sb=[n for n in st['nodes'] if not n['hidden'] and n['scrollable'] and n['rect'][1]>100 and n['rect'][2]-n['rect'][0]>=400 and n['rect'][3]-n['rect'][1]>=300]
    for _ in range(4):
        if not (seeall and seeall[0]['rect'][3]>620 and sb): break
        # Scroll from the left edge, clear of the rows' swipe-to-queue gesture.
        x,y,r,b=max(sb,key=lambda n:n['rect'][3]-n['rect'][1])['rect'];s.command('drag',30,b-20,30,y+120,.6);time.sleep(.6)
        st=s.snapshot();seeall=[n for n in st['nodes'] if n.get('text')=='See all songs' and not n['hidden']]
    s.check(seeall and seeall[0]['rect'][3]<=640,'"See all songs" scrolls into view above the mini-player')
    x,y,r,b=seeall[0]['rect'];s.command('click',(x+r)//2,(y+b)//2);time.sleep(1)
    full=s.capture('local-search-all-songs')
    s.check(full['route']=='collection','"See all songs" opens the shared collection list')
    s.label(full,'\u201cwithout\u201d')
    rows=[n for n in s.visible(full) if n.get('text')=='Without a Picture' and n['rect'][1]>100]
    s.check(len(rows)>6,'the full list holds more than the six instant results')
    s.click_label('Without a Picture');time.sleep(.6);ctx=s.snapshot()
    s.check(ctx['context_kind']=='SEARCH' and ctx['status']==1,'playing from the search list names the context SEARCH')
    s.check(not ctx['now_playing_open'],'search list playback stays in preview')
    s.setup('search-text zzzzzmissing');empty=s.capture('local-search-empty')
    s.check(not any(n.get('text')=='Caf\u00e9 Lights' for n in s.visible(empty)),'empty results discard previous search rows')
    # Clearing the field brings Browse back (2026-11-04 report: rows stayed).
    s.setup('search-text night');st=s.snapshot();s.check(any(n.get('text')=='Night Walk' for n in s.visible(st)),'a search shows rows')
    x,y,r,b=[n for n in s.visible(st) if n['clickable'] and n['rect'][1]<170 and n['rect'][0]>380][0]['rect'];s.command('click',(x+r)//2,(y+b)//2);time.sleep(.8)
    cl=s.capture('local-search-cleared');s.label(cl,'Browse all')
    s.check(not any(n.get('text') in ('Night Walk','Songs') and n['rect'][1]<640 for n in s.visible(cl)),'the clear button restores the browse tiles, no rows or chips left')
    s.command('click',320,80);time.sleep(.8);away=s.snapshot()  # the title row: away from the field
    s.check(not any(n.get('text')=='Night Walk' and n['rect'][1]<640 for n in s.visible(away)),'tapping away from an empty field keeps Browse')
    s.metrics()

def home_audiobooks(s,card):
    s.setup('playlist Night Drive');s.click_label('Night Walk')
    s.command('click',80,770);s.click_label('Music');before=s.snapshot()
    s.click_label('Audiobooks');home=s.capture('audiobooks-home')
    s.check(home['route']=='main' and home['tab']==0,'Audiobooks renders inside Home')
    for title in ('Music','Audiobooks','Your audiobooks','Continue listening','The Quiet Coast','Resume at 0:45','Home','Search','Your Library'):s.label(home,title)
    s.check(home['file']==before['file'] and home['status']==before['status'],'browsing books preserves music playback')
    s.click_label('The Quiet Coast');details=s.capture('audiobook-details')
    s.check(details['modal'] and details['file']==before['file'],'book opens contextual sheet without starting audio')
    s.click_label('Resume listening');time.sleep(1);playing=s.capture('audiobook-resumed')
    s.check(playing['file'].endswith('Audiobooks/book0.flac') and playing['status']==1 and 40<=playing['seconds']<60,'Resume restores saved audiobook file and position')
    s.check(not playing['now_playing_open'] and not playing['modal'],'Resume keeps audiobook feed and mini-player visible')
    s.click_label('Music');music=s.capture('music-after-audiobooks')
    s.label(music,'Night Drive');s.check(music['file']==playing['file'],'Music filter preserves audiobook playback')
    s.click_label('Audiobooks');s.command('drag',260,590,260,250);s.capture('audiobooks-shelves')
    s.command('click',240,770);s.command('click',80,770)
    # Home keeps its scroll position across a tab switch (Spotify does), so
    # the shelves are as left: still Audiobooks, scrolled to All audiobooks.
    back=s.snapshot();s.label(back,'All audiobooks')
    s.command('drag',260,250,260,650);s.settled();s.label(s.snapshot(),'Your audiobooks')
    s.check(s.snapshot()['file']==playing['file'],'return to Home remembers Audiobooks and playback')
    s.click_label('Scan books');time.sleep(2)
    scanned=s.capture('audiobooks-rescanned')
    s.label(scanned,'Your audiobooks');s.label(scanned,'The Quiet Coast')
    s.check(scanned['file']==playing['file'] and scanned['status']==1,'rescanning books preserves active playback')
    with sqlite3.connect('file:'+str(card/'.local/audiobooks.db')+'?mode=ro',uri=True) as db:
        s.check(db.execute('SELECT COUNT(*) FROM AUDIOBOOK_TABLE').fetchone()[0]==3,'rescan indexes only the three audiobooks, not music')
        s.check(db.execute('SELECT resume_pos FROM AUDIOBOOK_TABLE WHERE path=?',(str(card/'Audiobooks/book0.flac'),)).fetchone()[0]>=45,'rescan preserves saved book position')
    s.metrics()
    s.command('stop');s.command('start',3,'--card',card,'--preserve-config')
    restarted=s.capture('audiobooks-restarted');s.label(restarted,'Your audiobooks')
    s.check(restarted['tab']==0,'Audiobooks Home category survives restart')


def audiobook_player(s,card):
    """Audiobook Now Playing: book controls, and chapters, bookmarks and summary in the shared sheet."""
    green=lambda px:all(abs(a-b)<=10 for a,b in zip(px,(30,215,96)))
    s.setup('playlist Night Drive');s.click_label('Night Walk')
    s.command('click',80,770);s.click_label('Music');s.click_label('Audiobooks')
    s.click_label('The Quiet Coast');s.click_label('Resume listening');time.sleep(1)
    s.command('click',170,695);time.sleep(.6);np=s.capture('audiobook-now-playing')
    s.check(np['now_playing_open'] and np['file'].endswith('Audiobooks/book0.flac'),'mini-player expands the audiobook in Now Playing')
    s.label(np,'The Quiet Coast')
    s.check(not any(n.get('text','').startswith('PLAYING') for n in s.visible(np)),'the book header has no capitals line, as the music one')
    s.check(any(n.get('text','').endswith('x') and n['rect'][1]>560 for n in s.visible(np)),'book controls show the playback speed')
    def menu(item):
        s.command('click',436,56);time.sleep(.4);m=s.snapshot()
        s.check(m['modal'],'book menu opens in the shared sheet: '+item)
        s.click_label(item);time.sleep(.6)
    menu('Chapters');ch=s.capture('audiobook-chapters-sheet')
    s.check(ch['modal'] and ch['now_playing_open'],'Chapters opens as a sheet over Now Playing, not a separate page')
    for t in ('Chapters','Opening','The Harbour','Low Tide','Morning'):s.label(ch,t)
    img=Image.open(s.out/'audiobook-chapters-sheet.png').convert('RGB')
    # The sheet's row, not the Now Playing heading beneath the scrim.
    harbour=max((n for n in s.visible(ch) if n.get('text')=='The Harbour'),key=lambda n:n['id']);x,y,r,b=harbour['rect']
    s.check(any(green(img.getpixel((px,py))) for px in range(x,min(r,x+120)) for py in range(y,b)),'the chapter being heard is marked green')
    s.click_label('Low Tide');time.sleep(.8);st=s.snapshot()
    s.check(not st['modal'] and st['now_playing_open'],'picking a chapter closes the sheet onto Now Playing')
    s.check(60<=st['seconds']<70 and st['file'].endswith('book0.flac'),'picking a chapter seeks to its start')
    # The bottom shortcut is the chapter list on a book (the queue on music).
    s.command('click',440,755);time.sleep(.6);sc=s.capture('audiobook-chapters-shortcut')
    s.check(sc['modal'] and any(n.get('text')=='Morning' for n in s.visible(sc)),'the bottom shortcut opens the chapters sheet on a book')
    s.command('drag',240,65,240,450);time.sleep(.6)
    menu('Add bookmark');time.sleep(.4)
    menu('Bookmarks');bm=s.capture('audiobook-bookmarks-sheet')
    s.check(bm['modal'] and bm['now_playing_open'],'Bookmarks opens as a sheet over Now Playing')
    s.label(bm,'Bookmarks');s.label(bm,'Low Tide')
    s.command('drag',240,65,240,450);time.sleep(.6)
    s.check(not s.snapshot()['modal'],'sheet handle dismisses the bookmarks sheet')
    menu('Bookmarks');s.click_label('Low Tide');time.sleep(.8);st=s.snapshot()
    s.check(not st['modal'] and st['now_playing_open'] and 60<=st['seconds']<75,'picking a bookmark seeks there and returns to Now Playing')
    menu('Bookmarks');bm=s.snapshot()
    lab=max((n for n in s.visible(bm) if n.get('text')=='Low Tide'),key=lambda n:n['id']);y=(lab['rect'][1]+lab['rect'][3])//2
    rm=[n for n in s.visible(bm) if n['clickable'] and n['rect'][0]>380 and n['rect'][1]<=y<=n['rect'][3] and n['rect'][2]-n['rect'][0]==56]
    s.check(len(rm)==1,'each bookmark row offers a remove button')
    x,y0,r,b=rm[0]['rect'];s.command('click',(x+r)//2,(y0+b)//2);time.sleep(.6);gone=s.capture('audiobook-bookmarks-empty')
    s.check(gone['modal'] and not any(n.get('text')=='Low Tide' and n['rect'][1]>100 for n in s.visible(gone)),'removing the bookmark updates the sheet in place')
    s.check(any(n.get('text','').startswith('No bookmarks') for n in s.visible(gone)),'an empty bookmarks sheet says so')
    s.command('drag',240,65,240,450);time.sleep(.6)
    menu('Summary');sm=s.capture('audiobook-summary-sheet')
    s.check(sm['modal'] and sm['now_playing_open'],'Summary opens as a sheet over Now Playing')
    for t in ('Summary','The Quiet Coast','Mara Ellis','An original audiobook fixture for testing local listening.'):s.label(sm,t)
    s.command('drag',240,65,240,450);time.sleep(.6)
    # Playback speed: the "1x" label opens the titled sheet; the label follows the pick.
    s.click_label('1x');time.sleep(.5);sp=s.capture('audiobook-speed-sheet')
    s.check(sp['modal'],'speed opens in the shared sheet');s.label(sp,'Playback speed')
    s.click_label('1.5x');time.sleep(.5)
    s.check(any(n.get('text')=='1.5x' and n['rect'][1]>560 for n in s.visible(s.snapshot())),'the speed label shows the chosen 1.5x')
    s.click_label('1.5x');time.sleep(.5);s.click_label('1x');time.sleep(.5)
    s.check(any(n.get('text')=='1x' and n['rect'][1]>560 for n in s.visible(s.snapshot())),'speed back to 1x')
    # Sleep timer: the book's right-hand control, green while a stop is set.
    def moon():
        st=s.snapshot()
        return next(n for n in s.visible(st) if n['clickable'] and n['rect'][0]>400 and 580<n['rect'][1]<660 and n['rect'][2]-n['rect'][0]==56)
    def moon_green(name):
        x,y,r,b=moon()['rect'];img=Image.open(s.out/f'{name}.png').convert('RGB')
        return any(green(img.getpixel((px,py))) for px in range(x,r) for py in range(y,b))
    x,y,r,b=moon()['rect'];s.command('click',(x+r)//2,(y+b)//2);time.sleep(.5);sl=s.capture('audiobook-sleep-sheet')
    s.check(sl['modal'],'sleep timer opens in the shared sheet');s.label(sl,'Sleep timer')
    for t in ('5 minutes','30 minutes','1 hour','End of chapter'):s.label(sl,t)
    s.check(not any(n.get('text')=='Turn off timer' for n in s.visible(sl)),'no Turn off entry while no timer is set')
    s.click_label('End of chapter');time.sleep(.6);s.capture('audiobook-sleep-on')
    s.check(moon_green('audiobook-sleep-on'),'the moon turns green while a stop is set')
    x,y,r,b=moon()['rect'];s.command('click',(x+r)//2,(y+b)//2);time.sleep(.5)
    s.click_label('Turn off timer');time.sleep(.6);s.capture('audiobook-sleep-off')
    s.check(not moon_green('audiobook-sleep-off'),'turning the timer off clears the green moon')
    end=s.snapshot();s.check(end['status']==1 and end['file'].endswith('book0.flac'),'book sheets never interrupt playback')
    s.metrics()

def library_create(s,card):
    """Your Library: Create sheet and naming, playlist options, rename/delete, and every row swipe."""
    green=lambda px:all(abs(a-b)<=12 for a,b in zip(px,(30,215,96)))
    red=lambda px:all(abs(a-b)<=14 for a,b in zip(px,(233,20,41)))
    def top(st,text):
        return max((n for n in s.visible(st) if n.get('text')==text),key=lambda n:n['id'])
    def img(name):
        return Image.open(s.out/f'{name}.png').convert('RGB')
    def any_px(im,rect,test):
        x,y,r,b=rect;return any(test(im.getpixel((px,py))) for px in range(max(0,x),min(480,r),3) for py in range(max(0,y),min(800,b),3))
    s.setup('playlist Night Drive');s.click_label('Night Walk')
    s.command('click',400,770);time.sleep(.6);lib=s.capture('library-root')
    texts=[n.get('text','') for n in s.visible(lib)]
    s.check('...' not in texts,'library rows carry no "..." button (long press opens options)')
    rows=[n for n in s.visible(lib) if n['clickable'] and 150<n['rect'][1]<640 and n['rect'][3]-n['rect'][1]==80]
    s.check(rows and all(n['rect'][2]-n['rect'][0]>=400 for n in rows),'library rows span the list (%s)'%[n['rect'] for n in rows[:3]])
    s.check(any(t.startswith('Playlist \u00b7 ') for t in texts) and not any(' / ' in t for t in texts),'library subtitles use the Spotify middle dot')
    # Create sheet: compact, two described kinds, no stray Done button.
    s.command('click',434,78);time.sleep(.6);sheet=s.capture('create-sheet')
    s.check(sheet['modal'],'+ opens the Create sheet')
    for t in ('Playlist','Build a playlist with songs','Folder','Group your playlists together'):s.label(sheet,t)
    s.check(not any(n.get('text')=='Done' for n in s.visible(sheet)),'the Create sheet has no Done button over its close control')
    s.check(top(sheet,'Playlist')['rect'][1]>520,'the Create sheet rises only as far as its two rows')
    s.click_label('Playlist');time.sleep(.6);name=s.capture('create-playlist-name')
    for t in ('Give your playlist a name','Cancel','Create'):s.label(name,t)
    field=next(n for n in s.visible(name) if n['type']=='textarea')
    s.check(field['rect'][2]-field['rect'][0]>=380,'the name field is wide and centred')
    pills=[n for n in s.visible(name) if n.get('text') in ('Cancel','Create') and n['rect'][1]<400]
    s.check(len(pills)==2 and abs(pills[0]['rect'][1]-pills[1]['rect'][1])<4,'Cancel and Create pills sit side by side above the keyboard')
    im=img('create-playlist-name');cr=next(n for n in pills if n['text']=='Create')
    par=next(n for n in name['nodes'] if n['id']==cr['parent'])
    s.check(any_px(im,par['rect'],green),'Create is the green primary pill')
    # Create with the suggested name: the new empty playlist opens.
    x,y,r,b=par['rect'];s.command('click',(x+r)//2,(y+b)//2);time.sleep(1);made=s.capture('create-playlist-done')
    title=[n['text'] for n in s.visible(made) if n.get('text','').startswith('My playlist #')]
    s.check(made['route']=='collection' and title,'Create with the suggested name opens the new playlist')
    s.check(made['status']==1,'creating a playlist keeps playback')
    created=title[0]
    s.label(made,"Let's start building your playlist")
    add=top(made,'Add to this playlist');x,y,r,b=next(n for n in made['nodes'] if n['id']==add['parent'])['rect']
    s.command('click',(x+r)//2,(y+b)//2);time.sleep(.8);adding=s.capture('empty-playlist-add-songs')
    s.check(adding['modal'],'the empty playlist offers Add to this playlist, opening the song picker')
    s.command('click',448,80);time.sleep(.6)
    s.command('click',400,770);time.sleep(.6);lib=s.snapshot();row=top(lib,created)
    # Long press: the options sheet with icons, sized to its rows.
    y=(row['rect'][1]+row['rect'][3])//2;s.command('drag',240,y,241,y,1.2);time.sleep(.5);opts=s.capture('playlist-options-sheet')
    s.check(opts['modal'],'long press opens the playlist options')
    for t in ('Add songs','Rename','Delete playlist','Pin to top'):s.label(opts,t)
    s.check(not any(n.get('text')=='Done' for n in s.visible(opts)),'options sheet has no Done button')
    s.click_label('Rename');time.sleep(.6);ren=s.capture('playlist-rename')
    s.label(ren,'Rename playlist');s.label(ren,'Save')
    field=next(n for n in s.visible(ren) if n['type']=='textarea')
    s.click_label('Cancel');time.sleep(.6);s.check(not s.snapshot()['modal'],'Cancel closes the naming sheet')
    lib=s.snapshot();row=top(lib,created);y=(row['rect'][1]+row['rect'][3])//2
    s.command('drag',240,y,241,y,1.2);time.sleep(.5);s.click_label('Delete playlist');time.sleep(.6)
    ask=s.capture('playlist-delete-confirm');s.label(ask,'Delete playlist?');s.label(ask,'Delete')
    s.click_label('Delete');time.sleep(.8);gone=s.snapshot()
    s.check(not any(n.get('text')==created for n in s.visible(gone)),'Delete removes the playlist after confirmation')
    # Library swipe right pins (green strip), and again unpins.
    row=top(gone,'Night Drive');y=(row['rect'][1]+row['rect'][3])//2
    sub=[n for n in s.visible(gone) if n.get('text','').startswith('Playlist \u00b7') and row['rect'][1]<n['rect'][1]<row['rect'][1]+50][0]
    s.pointer('mousemove',150,y,'mousedown','1','sleep','.1','mousemove',180,y,'sleep','.05','mousemove',240,y,'sleep','.05','mousemove',290,y,'sleep','.25')
    mid=s.capture('library-swipe-armed');s.pointer('mouseup','1');time.sleep(.8)
    moved=top(mid,'Night Drive')
    s.check(moved['rect'][0]-row['rect'][0]>=100,'the library row content follows the swipe')
    rowbox=next(n for n in mid['nodes'] if n['id']==moved['parent'])
    s.check(any_px(img('library-swipe-armed'),[rowbox['rect'][0],rowbox['rect'][1],rowbox['rect'][0]+90,rowbox['rect'][3]],green),'past the threshold the strip is green')
    after=s.snapshot();sub2=[n for n in s.visible(after) if n.get('text','').startswith('Playlist \u00b7') and abs(n['rect'][1]-sub['rect'][1])<200]
    pinned=top(after,'Night Drive')
    s.check(pinned['rect'][0]==row['rect'][0] or pinned['rect'][0]-row['rect'][0]<4,'the row content returns after the swipe')
    s.check(any(n['type']=='image' and n['rect'][2]-n['rect'][0]==16 and abs(n['rect'][1]-pinned['rect'][3])<30 for n in s.visible(after)),'swipe right pins the playlist (pin mark)')
    p=top(after,'Night Drive');y=(p['rect'][1]+p['rect'][3])//2
    s.command('drag',150,y,300,y+2,.1);time.sleep(.8)
    # Song rows: swipe right queues past the threshold only.
    col=s.setup('playlist Night Drive');base=col['priority']
    # The header's pills push the songs down: bring the rows into view first.
    s.command('drag',240,560,240,300,.3);col=s.settled()
    row=s.label(col,'Caf\u00e9 Lights');y=(row['rect'][1]+row['rect'][3])//2
    s.command('drag',150,y,200,y+2,.05);time.sleep(.4)
    st=s.snapshot();s.check(st['priority']==base,'a short swipe springs back without queueing')
    s.check(s.label(st,'Caf\u00e9 Lights')['rect'][0]==row['rect'][0],'the short swipe leaves the row where it was')
    s.check(not st['now_playing_open'] and st['file']==col['file'],'a swipe never plays the row')
    s.pointer('mousemove',150,y,'mousedown','1','sleep','.1','mousemove',175,y,'sleep','.05','mousemove',230,y,'sleep','.05','mousemove',290,y,'sleep','.25')
    mid=s.capture('song-swipe-armed');s.pointer('mouseup','1');time.sleep(.8)
    moved=top(mid,'Caf\u00e9 Lights');s.check(moved['rect'][0]-row['rect'][0]>=100,'the song row slides with the finger')
    rowbox=next(n for n in mid['nodes'] if n['id']==moved['parent'])
    s.check(any_px(img('song-swipe-armed'),[rowbox['rect'][0],rowbox['rect'][1],rowbox['rect'][0]+90,rowbox['rect'][3]],green),'the song strip turns green when armed')
    st=s.snapshot();s.check(st['priority']==base+1,'releasing past the threshold adds the song to the queue')
    s.check(s.label(st,'Caf\u00e9 Lights')['rect'][0]==row['rect'][0],'the song row springs back into place')
    # A mostly vertical drag still scrolls the list and queues nothing.
    big=s.setup('playlist Large Playlist');pri=big['priority']
    s.command('drag',200,620,230,330,.3);time.sleep(.6);sc=s.snapshot()
    s.check(sc['priority']==pri,'a vertical drag never queues')
    # Queue sheet: swipe left removes an upcoming row over a red strip.
    s.setup('playlist Night Drive');s.click_label('Night Walk');time.sleep(.5)
    s.command('click',170,695);time.sleep(.6);s.command('click',440,755);time.sleep(.6);q=s.snapshot()
    before=len(q['queue'])
    target=max((n for n in s.visible(q) if n.get('text')=='Blue Hour' or n.get('text')=='First Light'),key=lambda n:n['id'],default=None)
    if target is None:
        names=[n['text'] for n in s.visible(q) if n.get('text') and n['rect'][1]>300]
        target=top(q,names[-1])
    y=(target['rect'][1]+target['rect'][3])//2
    s.pointer('mousemove',400,y,'mousedown','1','sleep','.1','mousemove',375,y,'sleep','.05','mousemove',320,y,'sleep','.05','mousemove',260,y,'sleep','.25')
    mid=s.capture('queue-swipe-armed');s.pointer('mouseup','1');time.sleep(.8)
    moved=next(n for n in mid['nodes'] if n['id']==target['id'])
    s.check(target['rect'][0]-moved['rect'][0]>=100,'the queue row slides left with the finger')
    rowbox=next(n for n in mid['nodes'] if n['id']==moved['parent'])
    s.check(any_px(img('queue-swipe-armed'),[rowbox['rect'][2]-90,rowbox['rect'][1],rowbox['rect'][2],rowbox['rect'][3]],red),'the queue strip turns red when armed')
    after=s.snapshot();s.check(len(after['queue'])==before-1,'releasing past the threshold removes the row from the queue')
    s.check(after['modal'] and after['status']==1,'the queue stays open and playback continues')
    s.metrics()

def home_cards(s,card):
    """Home and Search cards: Spotify shortcut tiles, peeking shelves and browse tiles, with card carousels."""
    green=lambda px:px[1]>150 and px[1]>px[0]+80 and px[1]>px[2]+40
    def nodes_by(st,pred):return [n for n in s.visible(st) if pred(n)]
    def wid(n):return n['rect'][2]-n['rect'][0]
    def hei(n):return n['rect'][3]-n['rect'][1]
    s.setup('playlist Night Drive');s.click_label('Night Walk');time.sleep(.5)
    s.command('click',80,770);time.sleep(.8);home=s.capture('cards-home')
    texts=[n.get('text','') for n in s.visible(home)]
    s.check('...' not in texts,'Home cards carry no "..." badges')
    tiles=nodes_by(home,lambda n:n['clickable'] and hei(n)==56 and wid(n)>=200 and n['rect'][1]<520)
    s.check(4<=len(tiles)<=8 and len({wid(n) for n in tiles})==1,'shortcut tiles: 56 px, equal width, up to eight')
    s.check(len({n['rect'][0] for n in tiles})==2,'shortcut tiles form two columns')
    s.label(home,'Liked Songs')
    lit=[n for n in s.visible(home) if n.get('text')=='Night Drive' and n['rect'][1]<520]
    img=Image.open(s.out/'cards-home.png').convert('RGB')
    s.check(lit and any(green(img.getpixel((x,y))) for n in lit[:1] for x in range(n['rect'][0],min(n['rect'][2],n['rect'][0]+100)) for y in range(n['rect'][1],n['rect'][3])),'the playing collection\'s tile title is green')
    # Scroll to the shelves and inspect a carousel.
    s.command('drag',240,600,240,330,.05);s.settled();shelfst=s.capture('cards-home-shelf')
    cards=nodes_by(shelfst,lambda n:n['clickable'] and wid(n)==150 and 100<n['rect'][1]<600)
    s.check(cards,'Home shelves show 150 px cards')
    rows={}
    for c in cards:rows.setdefault(c['parent'],[]).append(c)
    row_id,row_cards=max(rows.items(),key=lambda kv:len(kv[1]))
    row=next(n for n in shelfst['nodes'] if n['id']==row_id)
    s.check(row['rect'][0]==0 and row['rect'][2]==480,'the shelf runs from screen edge to screen edge')
    first=min(row_cards,key=lambda n:n['rect'][0])
    s.check(first['rect'][0]==20,'the first card lines up with the page gutter')
    s.check(any(c['rect'][0]<480<c['rect'][2] for c in row_cards),'the next card peeks in at the right edge')
    s.check(not any(n['clickable'] and wid(n)==42 and hei(n)==42 for n in s.visible(shelfst)),'shelf cards carry no overlay Play button')
    y=(first['rect'][1]+first['rect'][1]+150)//2
    s.command('drag',420,y,120,y+3,.05);moved=s.settled()
    after=[n for n in moved['nodes'] if n['parent']==row_id and wid(n)==150 and not n['hidden']]
    s.check(min(n['rect'][0] for n in after)<first['rect'][0]-100,'a sideways drag flicks the shelf along')
    s.check(any(n['rect'][0]==20 for n in after),'the shelf comes to rest with a card at the gutter')
    s.check(not moved['modal'] and moved['route']=='main' and moved['file']==home['file'],'flicking a shelf opens and plays nothing')
    yb=next(n for n in moved['nodes'] if n['id']==row_id)['rect'][1]
    s.command('drag',300,y,305,y-220,.05);vert=s.settled()
    s.check(next(n for n in vert['nodes'] if n['id']==row_id)['rect'][1]<yb-100,'a vertical drag on a shelf scrolls the page')
    # Long press offers the card menu in the shared sheet style.
    c=next(n for n in s.visible(vert) if n['clickable'] and wid(n)==150 and 120<n['rect'][1] and n['rect'][1]+150<640 and 0<=n['rect'][0]<330)
    s.command('drag',c['rect'][0]+70,c['rect'][1]+70,c['rect'][0]+71,c['rect'][1]+70,1.2);time.sleep(.5)
    menu=s.capture('cards-home-menu');s.check(menu['modal'],'long press opens the card menu')
    for t in ('Play collection','Add to queue'):s.label(menu,t)
    s.command('click',240,200);time.sleep(.6);s.check(not s.snapshot()['modal'],'a scrim tap closes the card menu')
    # Search: top genres carousel, then one Browse all grid.
    s.command('click',240,770);time.sleep(1);srch=s.capture('cards-search')
    s.label(srch,'Your top genres')
    gen=nodes_by(srch,lambda n:n['clickable'] and wid(n)==196 and hei(n)==112)
    s.check(len(gen)>=3,'top genres are 196 x 112 tiles: %d'%len(gen))
    grow=next(n for n in srch['nodes'] if n['id']==gen[0]['parent'])
    s.check(grow['rect'][0]==0 and grow['rect'][2]==480,'the genre carousel runs edge to edge')
    s.check(any(t['rect'][0]<480<t['rect'][2] for t in gen),'the next genre tile peeks in')
    gy=(gen[0]['rect'][1]+gen[0]['rect'][3])//2
    x0=min(t['rect'][0] for t in gen)
    s.command('drag',420,gy,140,gy+3,.05);g2=s.settled()
    after=[n for n in g2['nodes'] if n['parent']==grow['id'] and not n['hidden']]
    s.check(min(n['rect'][0] for n in after)<x0-100 and any(n['rect'][0]==20 for n in after),'genre tiles flick and rest at the gutter')
    s.command('drag',240,560,240,230,.05);s.settled();brw=s.capture('cards-search-browse')
    s.label(brw,'Browse all')
    titles=('All songs','Liked Songs','Albums','Artists','Playlists','Genres','Most played','Files & folders')
    for t in titles:s.label(brw,t)
    grid=nodes_by(brw,lambda n:n['clickable'] and wid(n)==214 and hei(n)==112)
    s.check(len(grid)>=4 and len({n['rect'][0] for n in grid})==2,'Browse all is a two-column grid of 214 x 112 tiles')
    s.check(not any(n.get('text')=='Browse the microSD card' for n in s.visible(brw)),'Files & folders is a tile, not a separate row')
    s.metrics()

def home_audiobooks_empty(s,card):
    s.command('click',80,770);s.click_label('Audiobooks')
    empty=s.capture('audiobooks-empty')
    s.label(empty,'Your next chapter starts here')
    s.check(empty['route']=='main' and empty['tab']==0 and not empty['file'],'empty Audiobooks remains in Home without starting playback')
    s.click_label('Scan books');time.sleep(2)
    scanned=s.capture('audiobooks-empty-scanned')
    s.label(scanned,'Your next chapter starts here');s.label(scanned,'Scan books')
    s.check(not scanned['modal'],'empty scan completes in place without a legacy screen')
    s.click_label('Music');s.check(s.snapshot()['route']=='main','empty category can return to Music')
    s.metrics()

def collection_find(s,card):
    s.setup('playlist Night Drive');initial=s.capture('find-playlist-initial')
    _,play=s.collection_geometry(initial);cy=(play[1]+play[3])//2
    s.command('click',43,cy);on=s.capture('shuffle-toggle-on')
    s.check(not on['file'] and not on['queue'],'Shuffle on does not start playback')
    img=Image.open(s.out/'shuffle-toggle-on.png').convert('RGB')
    s.check(any(g>r*1.4 and g>b*1.2 and g>100 for r,g,b in list(img.crop((25,cy-20,65,cy+20)).getdata())),'Shuffle enabled glyph is green')
    s.command('click',43,cy);off=s.snapshot();s.check(not off['file'],'Shuffle off is also passive')
    s.command('click',43,cy);s.command('click',(play[0]+play[2])//2,cy)
    playing=s.snapshot();s.check(playing['shuffle'] and len(playing['queue'])==6,'Play starts the collection with selected shuffle')
    s.check(not playing['now_playing_open'],'Play stays in preview')
    # Reveal via native downward drag at top, then edit the actual textarea.
    s.command('drag',220,240,220,410,.65);revealed=s.capture('find-revealed')
    fields=[n for n in s.visible(revealed) if n['type']=='textarea']
    s.check(len(fields)==1,'downward pull reveals one collection search field')
    x,y,r,b=fields[0]['rect'];s.command('click',(x+r)//2,(y+b)//2)
    s.click_label('c');time.sleep(.3)
    s.setup('collection-find cafe');filtered=s.capture('find-title')
    s.label(filtered,'Caf\u00e9 Lights')
    f=next(n for n in s.visible(filtered) if n['type']=='textarea');s.check(f['rect'][1]>=130 and f['rect'][3]<520,'typing field stays clear of heading and keyboard')
    s.check(filtered['file']==playing['file'] and filtered['status']==1,'filter typing preserves playback')
    s.check(not any(n.get('text')=='First Light' and 190<n['rect'][1]<520 for n in s.visible(filtered)),'title filter hides nonmatching tracks')
    s.setup('collection-find quiet quartet');artist=s.capture('find-artist');s.label(artist,'Without a Picture')
    s.setup('collection-find shared horizon');album=s.capture('find-album');s.label(album,'Guest Appearance')
    s.setup('collection-find no such recording');empty=s.capture('find-no-results');s.label(empty,'No matching songs')
    s.setup('collection-find cafe');s.click_label('Done');done=s.capture('find-keyboard-dismissed')
    s.click_label('Caf\u00e9 Lights');chosen=s.snapshot()
    s.check(chosen['file'].endswith('track001.flac') and len(chosen['queue'])==6,'filtered row plays the correct song in its full collection context')
    s.check(not chosen['now_playing_open'],'filtered playback keeps collection visible')
    s.click_label('\u00d7');cleared=s.snapshot();s.label(cleared,'Night Walk')
    s.click_label('Original');s.capture('find-title-sorted')
    s.click_label('Title');s.click_label('Artist');s.click_label('Album');added=s.capture('find-added-sorted');s.label(added,'Added')
    s.check(added['file']==chosen['file'],'view sorting preserves current audio')
    # Fast scrollbar travels to the real end without allocating more rows.
    large=s.setup('playlist Large Playlist');s.capture('fast-scroll-top')
    s.command('drag',468,150,468,635,.6);bottom=s.capture('fast-scroll-bottom')
    s.check(bottom['node_count']<=large['node_count']+25,'long playlist scrollbar keeps pooled rows bounded')
    s.check(any(n['rect'][1]<-19000 and n['rect'][3]<=663 for n in bottom['nodes']),'scrollbar drag reaches the last rows of the 257-track canvas')
    s.check(not any(n.get('text')=='Large Playlist' and n['rect'][1]>100 for n in s.visible(bottom)),'fast scrollbar reaches beyond the expanded header')
    s.command('drag',468,630,468,130,.6);top=s.capture('fast-scroll-return');s.collection_geometry(top)
    # Verify the same layout with and without embedded artwork.
    for index in (0,5,3):
        state=s.setup('album '+str(card/f'Music/track{index:03d}.flac'));s.capture('audit-album-'+str(index));s.collection_geometry(state)
    s.setup('album '+str(card/'Music/track000.flac'))
    s.command('drag',220,240,220,410,.65);s.setup('collection-find nova')
    album_find=s.capture('find-within-album');s.label(album_find,'Caf\u00e9 Lights')
    s.check(not any(n.get('text')=='First Light' and 190<n['rect'][1]<650 for n in s.visible(album_find)),'album filter excludes matching artist tracks on other releases')
    s.command('click',400,770);s.click_label('Liked Songs')
    s.command('drag',220,240,220,410,.65);liked=s.capture('find-liked-songs')
    s.check(any(n['type']=='textarea' for n in s.visible(liked)),'Liked Songs supports the same pull-down filter')
    s.metrics()

def week_start(day):return day-(day+3)%7

def week_dates(start):
    a=time.gmtime(start*86400);b=time.gmtime((start+6)*86400);m=lambda t:time.strftime('%b',t)
    return f'{m(a)} {a.tm_mday} – {b.tm_mday}' if a.tm_mon==b.tm_mon else f'{m(a)} {a.tm_mday} – {m(b)} {b.tm_mday}'

def seed_activity(card):
    # A history the pages can be checked against: today three plays from the
    # Night Drive playlist (one song twice), yesterday one with no known
    # context, nine days ago one from an album. Counted plays this week, in
    # the two weeks before and outside the kept month; 85 minutes listened.
    now=int(time.time());day=now//86400;ws=week_start(day)
    m=lambda i:str(card/'Music'/f'track{i:03d}.flac')
    with sqlite3.connect(str(card/'.local/library.db')) as db:
        db.execute('DELETE FROM PLAY_HISTORY');db.execute('DELETE FROM HOME_PLAYS');db.execute('DELETE FROM LISTEN_TIME')
        db.executemany('INSERT INTO PLAY_HISTORY(path,name,artist,played_at,ctx_kind,context) VALUES(?,?,?,?,?,?)',[
            (m(3),'First Light','Nova',now-9*86400,'ALBUM','First Light'),(m(1),'Café Lights','Night Ensemble; Nova',now-86400,'',''),
            (m(0),'Night Walk','Night Ensemble',now-90,'PLAYLIST','Night Drive'),(m(3),'First Light','Nova',now-60,'PLAYLIST','Night Drive'),
            (m(0),'Night Walk','Night Ensemble',now-30,'PLAYLIST','Night Drive')])
        db.executemany('INSERT INTO HOME_PLAYS VALUES(?,?,?)',[(m(0),day,4),(m(3),day,3),(m(0),ws-5,2),(m(1),ws-5,1),(m(3),ws-13,5),(m(5),day-40,9)])
        db.execute('INSERT INTO LISTEN_TIME VALUES(?,?)',(day,85*60+20))
        db.execute('DELETE FROM PLAY_COUNTS');db.executemany('INSERT INTO PLAY_COUNTS(path,plays,last_played) VALUES(?,?,?)',[(m(0),6,now-30),(m(3),3,now-60)])
        db.execute('DELETE FROM FAVOURITES')
        db.executemany('INSERT INTO FAVOURITES(path,name,artist,added_at) VALUES(?,?,?,?)',[(m(0),'Night Walk','Night Ensemble',now-500),(m(3),'First Light','Nova',now-400)])

def profile_activity(s,card):
    import re
    s.command('stop');seed_activity(card);s.command('start',3,'--card',card,'--preserve-config');time.sleep(1)
    day=int(time.time())//86400;ws=week_start(day)
    def footer(state,name):
        for t in ('Home','Search','Your Library'):s.label(state,t)
        s.check(any(n.get('text')=='Choose something to play' or n.get('text')=='Night Walk' for n in s.visible(state)) or state['file'],name+': mini-player stays mounted')
    def avatar():
        st=s.snapshot()
        btn=[n for n in s.visible(st) if n['clickable'] and n['rect'][2]-n['rect'][0]==44 and n['rect'][1]<140 and n['rect'][0]<60]
        s.check(len(btn)==1,'one round avatar beside the tab title')
        x,y,r,b=btn[0]['rect'];s.command('click',(x+r)//2,(y+b)//2);time.sleep(.5)
    def back():
        s.command('click',44,87);time.sleep(.8)  # the back disc
    def centred(state,title):
        n=s.label(state,title);s.check(abs((n['rect'][0]+n['rect'][2])/2-240)<=2,title+': title centred, as Spotify\'s pushed pages')
    s.command('click',80,770);home=s.snapshot()
    s.check(any(n.get('text')=='S' and n['rect'][0]<60 and n['rect'][1]<140 for n in s.visible(home)),'avatar shows the initial, as Spotify\'s default avatar')
    # The profile menu slides in from the left, about nine tenths of the width.
    avatar();drawer=s.capture('profile-drawer')
    s.check(drawer['modal'],'profile drawer opens over the page')
    for t in ('Sonix R1','View device info','Listening stats','Recents','Settings','Devices','Audiobooks','Browse music','More'):s.label(drawer,t)
    stats_row=s.label(drawer,'Listening stats')
    s.check(stats_row['rect'][0]<120 and s.label(drawer,'More')['rect'][2]<=432,'drawer is a left panel, not a bottom sheet')
    img=Image.open(s.out/'profile-drawer.png').convert('RGB')
    head=s.label(drawer,'Sonix R1');hy=(head['rect'][1]+head['rect'][3])//2
    s.check(sum(img.getpixel((300,hy)))>sum(img.getpixel((300,stats_row['rect'][3]+20)))+15,'the profile sits on a raised card')
    s.check(sum(img.getpixel((462,400)))<sum(img.getpixel((200,400)))+10 and sum(img.getpixel((462,400)))<60,'page beside the drawer is dimmed')
    s.command('click',460,400);time.sleep(.5);s.check(not s.snapshot()['modal'],'tapping the dimmed page closes the drawer')
    avatar();s.command('drag',300,500,40,505,.05);time.sleep(.6)
    s.check(not s.snapshot()['modal'],'dragging the drawer left closes it')
    # Recents: by day, what came from one place is one row that opens.
    avatar();s.click_label('Recents');time.sleep(.8);rec=s.capture('recents')
    s.check(not rec['modal'],'choosing Recents closes the drawer')
    for t in ('Recents','Today','Yesterday','Night Drive','2 songs played · Playlist','1 song played','Night Ensemble; Nova'):s.label(rec,t)
    centred(rec,'Recents')
    vis=s.visible(rec);y=lambda st,t:min(n['rect'][1] for n in s.visible(st) if n.get('text')==t)
    s.check(y(rec,'Today')<y(rec,'Night Drive')<y(rec,'Yesterday')<y(rec,'1 song played'),'groups sit under their day')
    s.check(not any(n.get('text')=='Night Walk' for n in vis),'a group starts folded')
    grp=s.label(rec,'Night Drive');s.command('click',240,(grp['rect'][1]+grp['rect'][3])//2);time.sleep(.6);opened=s.capture('recents-open')
    songs=[n for n in s.visible(opened) if n.get('text') in ('Night Walk','First Light') and y(opened,'Night Drive')<n['rect'][1]<y(opened,'Yesterday')]
    s.check(sorted(n['text'] for n in songs)==['First Light','Night Walk'],'the group opens to each song once (%r)'%[n['text'] for n in songs])
    s.check(y(opened,'Night Walk')<y(opened,'First Light'),'newest play first')
    s.check(min(n['rect'][0] for n in songs)>grp['rect'][0],'the songs are indented under their group')
    footer(rec,'Recents')
    time.sleep(1.5);rec=s.capture('recents-art')
    img=Image.open(s.out/'recents-art.png').convert('RGB');row=s.label(rec,'Night Drive')
    art=[img.getpixel((x,(row['rect'][1]+row['rect'][3])//2)) for x in range(24,84,4)]
    s.check(any(max(p)-min(p)>40 for p in art),'Recents groups load their cover art')
    row=s.label(rec,'Night Walk');x,yy,r,b=row['rect'];s.command('click',300,(yy+b)//2);time.sleep(1);playing=s.snapshot()
    s.check(playing['status']==1 and playing['file'].endswith('track000.flac'),'tapping a song in a group plays it')
    s.check(not playing['now_playing_open'],'Recents playback stays in preview')
    s.command('click',240,(grp['rect'][1]+grp['rect'][3])//2);time.sleep(.5)
    s.check(not any(n.get('text')=='Night Walk' and n['rect'][1]<y(s.snapshot(),'Yesterday') for n in s.visible(s.snapshot())),'the group folds again')
    # Listening stats: this week's tiles, then the weeks before.
    s.command('click',80,770);avatar();s.click_label('Listening stats');time.sleep(1.2);st=s.capture('listening-stats')
    for t in ('Listening stats','This week',week_dates(ws),'Minutes listened','85','Top artist','Night Ensemble','Top song','Night Walk','Past weeks'):s.label(st,t)
    centred(st,'Listening stats')
    s.check(y(st,'Top artist')>y(st,'Minutes listened') and s.label(st,'Top song')['rect'][0]>200,'tiles in two columns: minutes and artist left, song right')
    s.command('drag',240,600,240,250,.05);st=s.settled();s.capture('listening-stats-past')
    weeks=[n for n in s.visible(st) if n.get('text') in (week_dates(ws-7),week_dates(ws-14))]
    s.check([n['text'] for n in sorted(weeks,key=lambda n:n['rect'][1])]==[week_dates(ws-7),week_dates(ws-14)],'past weeks, newest first')
    s.check(any(n.get('text')=='2 plays' for n in s.visible(st)) and any(n.get('text')=='5 plays' for n in s.visible(st)),'each past week gives its top song\'s plays')
    wk=s.label(st,week_dates(ws-14));s.command('click',240,(wk['rect'][1]+wk['rect'][3])//2);time.sleep(1);past=s.capture('listening-week-past')
    s.label(past,'First Light');s.check(any(n.get('text','').endswith('5 plays') for n in s.visible(past)),'a past week opens its ranked songs')
    back();st=s.snapshot();s.label(st,'This week');s.label(st,'Minutes listened')
    s.command('drag',240,250,240,650,.05);st=s.settled()
    tile=s.label(st,'Top song');s.command('click',(tile['rect'][0]+tile['rect'][2])//2,tile['rect'][3]+40);time.sleep(1);wkst=s.capture('listening-week')
    for t in (week_dates(ws),'Top artists','Top songs','1. Night Ensemble'):s.label(wkst,t)
    s.check(any(n.get('text')=='4 plays' for n in s.visible(wkst)) and any(n.get('text')=='3 plays' for n in s.visible(wkst)),'this week\'s artists ranked with their plays (4 and 3)')
    s.check(s.label(wkst,'Top artists')['rect'][3]-s.label(wkst,'Top artists')['rect'][1]>=28,'section headings are not clipped')
    s.command('drag',240,600,240,250,.05);wkst=s.settled()
    songs=[n for n in s.visible(wkst) if n.get('text') in ('Night Walk','First Light') and n['rect'][1]<650]
    order=[n['text'] for n in sorted(songs,key=lambda n:n['rect'][1])]
    s.check(order[:2]==['Night Walk','First Light'],'top songs in play order (%r)'%order)
    row=s.label(wkst,'First Light')
    s.command('click',240,(row['rect'][1]+row['rect'][3])//2);time.sleep(1);playing=s.snapshot()
    s.check(playing['file'].endswith('track003.flac') and playing['context_name']=='Your top songs','a top song plays the ranked list')
    s.check(not playing['now_playing_open'],'stats playback stays in preview')
    footer(playing,'Listening stats')
    # Settings: quick tiles, then Spotify's categories with icons and chevrons.
    s.command('click',80,770);avatar();s.click_label('Settings');time.sleep(.8);root=s.capture('settings-categories')
    centred(root,'Settings')
    for t in ('Wi-Fi','Bluetooth','Private session'):s.label(root,t)
    cats=('Content and display','Playback','Audio quality','Your Library','Apps and devices','Controls')
    for t in cats:s.label(root,t)
    rows=[n for n in s.visible(root) if n['clickable'] and n['rect'][2]-n['rect'][0]>=400 and 200<n['rect'][1]<640 and n['rect'][3]-n['rect'][1]<=120 and n['rect'][3]-n['rect'][1]!=84]
    s.check(rows and all(n['rect'][3]-n['rect'][1]==72 for n in rows),'category rows are 72 px')
    s.check(any('·' in n.get('text','') for n in s.visible(root)),'categories carry a summary line')
    imgs=[n for n in s.visible(root) if n['type']=='image']
    def inside(n,r):return r['rect'][1]<=(n['rect'][1]+n['rect'][3])//2<=r['rect'][3]
    whole=[r for r in rows if r['rect'][3]<640]
    s.check(len(whole)>=4 and all(any(inside(n,r) and n['rect'][0]<60 for n in imgs) and any(inside(n,r) and n['rect'][0]>400 for n in imgs) for r in whole),'category rows carry an icon and a chevron, as Spotify\'s')
    # Private session: nothing played while it is on reaches the history.
    db=sqlite3.connect(str(card/'.local/library.db'));count=lambda:db.execute('SELECT COUNT(*) FROM PLAY_HISTORY').fetchone()[0]
    s.click_label('Private session');time.sleep(.4);priv=s.capture('settings-private')
    tile=s.label(priv,'Private session');on=[n for n in s.visible(priv) if n.get('text')=='On' and abs(n['rect'][0]-tile['rect'][0])<80]
    s.check(on,'Private session tile turns On')
    single=str(card/'Music/track003.flac')
    # The song playing before the session began is still written down when it
    # ends; only what plays inside the session is kept out.
    s.setup('album '+single);s.click_label('First Light');time.sleep(2);before=count();s.snapshot('end-track');time.sleep(1.5);s.snapshot('end-track');time.sleep(1)
    s.check(count()==before,'a private session leaves no history (%d -> %d)'%(before,count()))
    s.command('click',80,770);avatar();s.click_label('Settings');time.sleep(.8);s.click_label('Private session');time.sleep(.4)
    s.check(any(n.get('text')=='Off' for n in s.visible(s.snapshot())),'Private session turns off again')
    db.close()
    # Search settings: the magnifier finds a setting by name or by what it does.
    st=s.snapshot();btn=[n for n in s.visible(st) if n['clickable'] and n['rect'][2]-n['rect'][0]==56 and n['rect'][0]>380 and n['rect'][1]<140]
    s.check(len(btn)==1,'Settings has a search button at the top right')
    x,yy,r,b=btn[0]['rect'];s.command('click',(x+r)//2,(yy+b)//2);time.sleep(.8);find=s.capture('settings-search')
    for t in ('Search','Search settings','Find settings by name or description'):s.label(find,t)
    s.snapshot('field-text equalizer');time.sleep(.5);hits=s.capture('settings-search-hits')
    s.label(hits,'Audio quality');s.check(not any(n.get('text')=='Find settings by name or description' for n in s.visible(hits)),'a query lists matching settings')
    s.snapshot('field-text crossfade');time.sleep(.5);hit=s.snapshot();s.label(hit,'Crossfade and gapless')
    s.click_label('Crossfade and gapless');time.sleep(.8);s.label(s.snapshot(),'Crossfade and gapless')
    back();back()
    # Storage (R1), found through Search settings.
    x,yy,r,b=btn[0]['rect'];s.command('click',(x+r)//2,(yy+b)//2);time.sleep(.8)
    s.snapshot('field-text storage');time.sleep(.5);s.click_label('Storage');time.sleep(1)
    # The card is walked on a thread of its own; the breakdown replaces the
    # "working out" line when it is done.
    for _ in range(20):
        sto=s.snapshot()
        if any(n.get('text')=='Music' for n in s.visible(sto)):break
        time.sleep(.5)
    sto=s.capture('settings-storage')
    for t in ('Storage','Card','Music','Audiobooks','Photos','Books','Player data'):s.label(sto,t)
    s.check(any(n.get('text','').endswith(' used') for n in s.visible(sto)),'Storage says how much of the card is used')
    s.check(any(n.get('text','') and n['text'].split(' ')[-1] in ('KB','MB','GB') and n['rect'][0]>300 for n in s.visible(sto)),'each kind shows its size')
    back();back()
    s.click_label('Playback');time.sleep(.8);pb=s.capture('settings-playback')
    for t in ('Playback','Crossfade and gapless','Queue and playback','Now Playing'):s.label(pb,t)
    s.command('drag',240,600,240,300,.3);s.label(s.settled(),'Shuffle by album')
    s.command('drag',20,400,440,410,.1);time.sleep(.8);s.click_label('Audio quality');time.sleep(.8);aq=s.capture('settings-audio-quality')
    s.label(aq,'Audio quality')
    s.check(not any(n.get('text') in ('Scan options','Queue and playback') for n in s.visible(aq)),'Audio quality holds the sound only')
    s.command('drag',20,400,440,410,.1);time.sleep(.8);s.click_label('Your Library');time.sleep(.8);lib=s.snapshot()
    for t in ('Scan options','Library'):s.label(lib,t)
    # Library health (R1): read-only counts with examples.
    s.click_label('Library health');time.sleep(.8);hl=s.capture('library-health')
    for t in ('Library health','Missing artist or album','No genre','Possible duplicates','Lossy files'):s.label(hl,t)
    s.check(all(any(re.fullmatch(r'None|1 song|\d+ songs',n.get('text','')) for n in s.visible(hl) if abs(n['rect'][1]-s.label(hl,k)['rect'][1]-30)<12) for k in ('No genre','Lossy files')),'each health row gives a count')
    s.click_label('No genre');time.sleep(.6);s.check(s.snapshot()['modal'],'a health row opens its examples')
    s.command('click',240,90);time.sleep(.5);s.command('click',44,87);time.sleep(.8)
    s.check(s.snapshot()['status']==1,'settings journey never interrupts playback')
    # Autoplay: on by default; when the context runs out, similar songs follow.
    s.command('drag',20,400,440,410,.1);time.sleep(.8);s.click_label('Playback');time.sleep(.8);pb=s.capture('settings-autoplay')
    sw=[n for n in s.visible(pb) if n['type']=='switch']
    sw=sorted(sw,key=lambda n:n['rect'][1])  # Autoplay first, then Shuffle by album
    s.check(len(sw)>=1 and sw[0].get('checked'),'Playback offers Autoplay, on by default')
    s.label(pb,'Keep listening to similar songs from your library when your music ends.')
    single=str(card/'Music/track003.flac')
    def play_single():
        s.setup('album '+single);s.click_label('First Light');time.sleep(1)
        st=s.snapshot();s.check(st['file']==single and st['status']==1,'the one-song release plays');return st
    play_single();s.snapshot('end-track');time.sleep(1);auto=s.snapshot()
    s.check(auto['status']==1 and auto['file']!=single and auto['file'].startswith(str(card/'Music')),'Autoplay continues with another local song (%s)'%auto['file'])
    s.check(auto['context_name']=='Autoplay','the queue says it is Autoplay')
    s.check(not auto['now_playing_open'],'Autoplay does not open Now Playing')
    s.command('click',80,770);avatar();s.click_label('Settings');time.sleep(.8);s.click_label('Playback');time.sleep(.8)
    sw=[n for n in s.visible(s.snapshot()) if n['type']=='switch'][0];x,y,r,b=sw['rect'];s.command('click',(x+r)//2,(y+b)//2)
    s.check(not next(n for n in s.snapshot()['nodes'] if n['id']==sw['id']).get('checked'),'Autoplay switches off')
    play_single();s.snapshot('end-track');time.sleep(1);end=s.snapshot()
    s.check(end['status']!=1 or end['file']==single,'with Autoplay off the queue stops at its end')
    s.command('stop');s.command('start',3,'--card',card,'--preserve-config');time.sleep(1)
    s.command('click',80,770);avatar();s.click_label('Settings');time.sleep(.8);s.click_label('Playback');time.sleep(.8)
    sw=[n for n in s.visible(s.snapshot()) if n['type']=='switch'][0]
    s.check(not sw.get('checked'),'Autoplay setting survives restart')
    # Home: an artist shelf for the artist played most (Spotify's "Discover more from").
    s.command('click',80,770);time.sleep(1);seen=set()
    for _ in range(8):
        hs=s.snapshot();seen|={n.get('text','') for n in s.visible(hs)}
        if any(n.get('text')=='Discover more from' and n['rect'][3]<380 for n in s.visible(hs)):break
        s.command('drag',240,560,240,240,.3);s.settled()
    hs=s.capture('home-artist-shelf');over=s.label(hs,'Discover more from')
    s.check(any(re.match(r'(Sun|Mon|Tues|Wednes|Thurs|Fri|Satur)day (mor|aft|eve|nig)',x) for x in seen),'Made for you leads with a Daylist named for the moment %r'%sorted(x for x in seen if x)[:40])
    s.check(any(n.get('text') in ('Night Ensemble','Nova') and 0<n['rect'][1]-over['rect'][1]<40 for n in s.visible(hs)),'the shelf names the most played artist under "Discover more from"')
    s.check(any(n.get('text')=='Album' and n['rect'][1]>over['rect'][3] for n in s.visible(hs)),'the shelf lists the artist albums')
    # Liked Songs genre chips (Spotify): a chip opens the liked songs of a genre.
    with sqlite3.connect(str(card/'.local/library.db')) as db:
        g=db.execute("SELECT genre FROM MEDIA_TABLE WHERE path=?",(str(card/'Music/track000.flac'),)).fetchone()[0]
    s.command('click',400,770);time.sleep(1);s.click_label('Liked Songs');time.sleep(1.2);lk=s.capture('liked-genres')
    chip=[n for n in s.visible(lk) if n.get('text')==g and n['rect'][1]<640]
    s.check(chip,'Liked Songs offers its genres as chips (%s)'%g)
    x,y,r,b=chip[0]['rect'];s.command('click',(x+r)//2,(y+b)//2);time.sleep(1.2);lg=s.capture('liked-genre-list')
    s.check(any(n.get('text')=='Night Walk' for n in s.visible(lg)),'a genre chip lists the liked songs of that genre')
    s.command('click',44,87);time.sleep(.8)  # back to Liked Songs; the Library tab follows
    # Smart Filters: Your Library's filter button lists genres with counts.
    import re
    s.command('click',400,770);time.sleep(1);lib=s.snapshot()
    fb=[n for n in s.visible(lib) if n['clickable'] and n['rect'][2]-n['rect'][0]==42 and n['rect'][3]-n['rect'][1]==42 and n['rect'][1]<220]
    s.check(len(fb)==1,'Your Library has a round filter button before its chips')
    x,y,r,b=fb[0]['rect'];s.command('click',(x+r)//2,(y+b)//2);time.sleep(.8);sheet=s.capture('library-filter-sheet')
    for t in ('Filter by','Genres'):s.label(sheet,t)
    genres=[n for n in s.visible(sheet) if re.fullmatch(r'.+ \(\d+\)',n.get('text',''))]
    s.check(genres,'the sheet lists genres with counts (%r)'%[n['text'] for n in genres][:4])
    name=genres[0]['text'].rsplit(' (',1)[0];x,y,r,b=genres[0]['rect'];s.command('click',(x+r)//2,(y+b)//2);time.sleep(1);fl=s.capture('library-filtered')
    s.check(any(n.get('text','')==name and n['rect'][1]<160 for n in s.visible(fl)),'the chosen genre shows as a chip')
    s.check(not any(n.get('text')=='Liked Songs' for n in s.visible(fl)),'shortcuts step aside while a genre filters the list')
    chip=next(n for n in s.visible(fl) if n.get('text','')==name and n['rect'][1]<160);x,y,r,b=chip['rect'];s.command('click',(x+r)//2,(y+b)//2);time.sleep(1)
    s.check(any(n.get('text')=='Liked Songs' for n in s.visible(s.snapshot())),'tapping the genre chip clears the filter')
    # Recommended songs under a playlist: library songs it does not hold, (+) adds one.
    s.setup('playlist Night Drive')
    # Small steps: a drag past the end of the list pulls the player up.
    for _ in range(6):
        if any(n.get('text')=='Refresh' and n['rect'][3]<650 for n in s.visible(s.snapshot())):break
        s.command('drag',240,520,240,300,.3);s.settled()
    rec=s.settled();s.capture('playlist-recommended')
    for t in ('Recommended songs','Based on the songs of this playlist','Refresh'):s.label(rec,t)
    rows=lambda:sqlite3.connect(str(card/'.local/library.db')).execute('SELECT COUNT(*) FROM "M3U_Night Drive"').fetchone()[0]
    before=rows();head=s.label(rec,'Based on the songs of this playlist')
    plus=[n for n in s.visible(rec) if n['clickable'] and n['rect'][2]-n['rect'][0]==52 and n['rect'][1]>head['rect'][3] and n['rect'][0]>380]
    s.check(plus,'each recommended song has an add button')
    x,y,r,b=plus[0]['rect'];s.command('click',(x+r)//2,(y+b)//2);time.sleep(1)
    s.check(rows()==before+1,'(+) adds the recommended song to the playlist (%d -> %d)'%(before,rows()))
    # Playlist cover (Spotify's Name & details): chosen from the playlist's albums.
    col=s.setup('playlist Night Drive');_,play=s.collection_geometry(col);cy=(play[1]+play[3])//2
    s.command('click',102,cy);time.sleep(.6);s.click_label('Choose cover');time.sleep(.6);cv=s.capture('playlist-cover-sheet')
    s.label(cv,'First song');albums=[n for n in s.visible(cv) if n.get('text') in ('Night Lights','First Light','A Shared Horizon','Unillustrated')]
    s.check(albums,'the cover sheet lists the playlist albums')
    x,y,r,b=albums[-1]['rect'];s.command('click',(x+r)//2,(y+b)//2);time.sleep(.6)
    with sqlite3.connect(str(card/'.local/library.db')) as db:cover=db.execute("SELECT path FROM PLAYLIST_COVER WHERE playlist='Night Drive'").fetchone()
    s.check(cover is not None,'the chosen album becomes the playlist cover')
    # Smart playlists (R1): from Create, filled by a rule, again on each open.
    def create_smart(rule,value=None):
        s.command('click',400,770);time.sleep(1);s.command('click',433,77);time.sleep(.6);s.click_label('Smart playlist');time.sleep(.6)
        s.click_label(rule);time.sleep(.6)
        if value:s.snapshot('field-text '+value);time.sleep(.3);s.click_label('Create');time.sleep(.8)
    db=lambda:sqlite3.connect(str(card/'.local/library.db'))
    create_smart('Most played')
    with db() as d:mp=d.execute('SELECT COUNT(*) FROM "M3U_Most played"').fetchone()[0];rule=d.execute("SELECT rule FROM SMART_PLAYLIST WHERE name='Most played'").fetchone()
    s.check(rule is not None and mp>0,'a Most played smart playlist fills from the play counts (%d)'%mp)
    create_smart('Genre is...','Ambient')
    with db() as d:amb=d.execute('SELECT COUNT(*) FROM "M3U_Ambient songs"').fetchone()[0]
    s.check(amb>=1,'a genre smart playlist holds that genre (%d)'%amb)
    s.check(any(n.get('text')=='Ambient songs' for n in s.visible(s.snapshot())),'the smart playlist is in Your Library')
    s.metrics()

def display_size(s,card):
    """Display size (R1, 2026-11-05): Large lays the interface out at 384x640 and
    draws it up to the 480x800 panel; the choice is saved for the next start and
    a restart is offered. Home is not rebuilt on a return when nothing changed."""
    log=s.repo/'artifacts/offline-ui/simulator.log'
    builds=lambda:log.read_text(errors='replace').count('home: feed built')
    def root(st):return next(n for n in st['nodes'] if n['parent']==-1)
    st=s.snapshot();r=root(st)['rect']
    s.check(r[2]-r[0]==480 and r[3]-r[1]==800,'Standard lays out pixel for pixel (%s)'%r)
    # Home is kept on a return: open Your Library and come back. First past the
    # startup card check, whose run is a legitimate reason to rebuild once.
    # One round trip brings Home up to date with that run; the second must not
    # rebuild it.
    time.sleep(6);s.click_label('Your Library');time.sleep(.8);s.click_label('Home');time.sleep(.8)
    before=builds();s.click_label('Your Library');time.sleep(.8);s.click_label('Home');time.sleep(.8)
    s.check(builds()==before,'returning to Home does not rebuild its feed when nothing changed')
    # Settings > Content and display > Appearance > Display size: Large.
    btn=[n for n in s.visible(s.snapshot()) if n['clickable'] and n['rect'][2]-n['rect'][0]==44 and n['rect'][1]<140 and n['rect'][0]<60]
    s.click_node(btn[0]);time.sleep(.6);s.click_label('Settings');time.sleep(.8)
    s.click_label('Content and display');time.sleep(.8);s.click_label('Appearance');time.sleep(.8)
    for _ in range(6):
        st=s.snapshot();head=[n for n in s.visible(st) if n.get('text')=='Display size']
        if head and head[0]['rect'][1]<560:break
        s.command('drag',240,640,240,360,.2);s.settled()
    st=s.capture('display-size-row');head=s.label(st,'Display size')
    s.label(st,'Standard');note=[n for n in s.visible(st) if (n.get('text') or '').startswith('Large makes everything')]
    s.check(note,'the Display size row explains what Large does and that it restarts')
    large=[n for n in s.visible(st) if n.get('text')=='Large' and n['rect'][1]>head['rect'][1]]
    s.check(large,'Display size offers Large');s.click_node(large[0]);time.sleep(.6)
    ask=s.capture('display-size-restart');s.label(ask,'Restart to change the display size?');s.label(ask,'Restart')
    s.click_label('Cancel');time.sleep(.5)
    ini=s.repo/'artifacts/offline-ui/device_config.ini'
    s.check('zoom = 125' in ini.read_text() or 'zoom=125' in ini.read_text(),'Large is saved for the next start')
    # The next start, at the saved size.
    s.command('stop');s.env['SONIX_ZOOM']='';s.zoom=1.25
    s.command('start',3,'--card',card,'--preserve-config');time.sleep(1.5)
    st=s.capture('display-large-home');r=root(st)['rect']
    s.check(r[2]-r[0]==384 and r[3]-r[1]==640,'Large lays out at 384x640 (%s)'%r)
    img=Image.open(s.out/'display-large-home.png');s.check(img.size==(480,800),'and fills the 480x800 panel (%s)'%(img.size,))
    for t in ('Home','Search','Your Library'):s.label(st,t)
    s.click_label('Search');time.sleep(.8);s.capture('display-large-search')
    s.click_label('Your Library');time.sleep(.8);lib=s.capture('display-large-library');s.label(lib,'Your Library')
    s.click_label('Liked Songs');time.sleep(1.2);s.capture('display-large-liked')
    s.command('drag',240,200,240,700,.1);time.sleep(.6);s.click_label('Search');time.sleep(.8)
    se=s.snapshot();clear=[n for n in s.visible(se) if n['clickable'] and n['rect'][1]<200 and n['rect'][0]>300 and n['rect'][2]-n['rect'][0]==44]
    s.check(not clear,'an empty search field shows no clear button')
    s.metrics()

def main():
    p=argparse.ArgumentParser();p.add_argument('repo',type=Path);p.add_argument('--baseline',type=Path);p.add_argument('--card',type=Path)
    p.add_argument('--journey',choices=['collections','preview-local','home-play','device-settings','player-core','local-search','home-audiobooks','home-audiobooks-empty','collection-find','audiobook-player','library-create','home-cards','profile-activity','display-size'],default='collections')
    a=p.parse_args();repo=a.repo.resolve();out=Path(tempfile.mkdtemp(prefix='ui-local-',dir=repo/'artifacts'))
    s=Session(repo,out,a.baseline);started=False
    try:
        if a.journey=='home-audiobooks-empty' and not a.card:
            card=out/'sd';(card/'Audiobooks').mkdir(parents=True)
        else:card=a.card.resolve() if a.card else create(repo,out/'sd')
        s.command('start',3,'--card',card);started=True
        {'collections':journey,'preview-local':preview_local,'home-play':home_play,'device-settings':device_settings,'player-core':player_core,'local-search':local_search,'home-audiobooks':home_audiobooks,'home-audiobooks-empty':home_audiobooks_empty,'collection-find':collection_find,'audiobook-player':audiobook_player,'library-create':library_create,'home-cards':home_cards,'profile-activity':profile_activity,'display-size':display_size}[a.journey](s,card)
        s.report['result']='PASS'
    except Exception as e:
        s.report['result']='FAIL';s.report['error']=repr(e)
        if started:
            try:s.capture('failure')
            except Exception:pass
        raise
    finally:
        try:
            if started:s.command('stop')
        finally:
            (out/'report.json').write_text(json.dumps(s.report,indent=2));print('UI report:',out/'report.json',flush=True)

if __name__=='__main__':main()
