#!/usr/bin/env python3
"""Real LVGL/SDL regression journey. Run under WSL after building the host.

Writes candidate screenshots + live state + metrics. Optional --baseline DIR
compares normalized captures; baseline images are never created or overwritten.
"""
import argparse
import json
import os
from pathlib import Path
import subprocess
import sqlite3
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
    def label(self,state,text):
        return next(n for n in self.visible(state) if n.get('text')==text)
    def click_label(self,text):
        node=self.label(self.snapshot(),text);x,y,r,b=node['rect'];self.command('click',(x+r)//2,(y+b)//2)
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
                         'playlist-preview','online-track-details','online-downloads'}
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
    s.label(playlist,'Playlist · 6 songs')
    s.check(not playlist['file'],'opening collections does not start playback')
    # Open playlist actions from the shared header, then dismiss by scrim.
    _,play=pg;cy=(play[1]+play[3])//2
    s.command('click',102,cy);menu=s.capture('playlist-menu');s.check(menu['modal'],'playlist action sheet opens')
    s.command('click',240,48);s.check(not s.snapshot()['modal'],'scrim dismisses sheet')
    s.command('click',102,cy);s.command('drag',240,74,240,285)
    s.check(not s.snapshot()['modal'],'handle swipe dismisses downward')
    empty=s.setup('playlist Empty Playlist');s.capture('playlist-empty');s.label(empty,'Use the menu to add songs')
    _,emptyplay=s.collection_geometry(empty)
    s.command('click',(emptyplay[0]+emptyplay[2])//2,(emptyplay[1]+emptyplay[3])//2)
    s.check(not s.snapshot()['file'],'empty playlist play cannot replace playback')
    longname='A very long playlist title for evenings beside the ocean and quiet mornings'
    long=s.setup('playlist '+longname);s.capture('playlist-long')
    title=next(n for n in s.visible(long) if n.get('text','').startswith('A very long playlist title'))
    _,longplay=s.collection_geometry(long)
    s.check(title['rect'][2]<=480 and title['rect'][3]<longplay[1],'long title fits above controls')
    large=s.setup('playlist Large Playlist');s.capture('playlist-large');s.label(large,'Playlist · 257 songs')
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
    row=s.label(edit,'Café Lights');parent=edit['nodes'][row['parent']];y=(parent['rect'][1]+parent['rect'][3])//2
    s.command('drag',429,y,429,y+84,.65)
    s.check(entries('Night Drive')==[original[0],original[2],original[1],*original[3:]],'drag reorder persists exact playlist order')
    s.check(not s.snapshot()['modal'],'row dragging does not open or dismiss a sheet')
    # Add a song to the empty collection through the existing local selector.
    s.setup('playlist Empty Playlist');s.command('click',102,cy);s.click_label('Add songs')
    s.check(s.snapshot()['modal'],'nested Add songs stays in the modal flow')
    s.click_label('Café Lights')
    s.check(entries('Empty Playlist')==[str(card/'Music/track001.flac')],'song selector persists playlist membership')
    s.command('click',448,80);updated=s.capture('playlist-added');s.label(updated,'Playlist · 1 song')
    s.check(updated['file']==before['file'],'adding songs does not replace playback')
    # Queue insertion and shuffle use real controls, observed independently.
    s.setup('playlist Night Drive');s.command('click',102,cy);s.click_label('Add to queue')
    queued=s.snapshot();s.check(queued['priority']==6,'playlist menu adds six manual priority entries')
    # Existing Sonix success notices intercept a tap until dismissed.
    s.command('click',240,48)
    s.command('click',43,cy);shuffled=s.snapshot()
    s.check(shuffled['shuffle'] and shuffled['priority']==6,'collection shuffle preserves priority entries')
    s.check(not shuffled['now_playing_open'],'collection Shuffle stays in mini-player preview')
    s.command('click',400,770);s.click_label('Playlists');filtered=s.snapshot()
    s.check(filtered['route']=='main' and filtered['tab']==2 and filtered['library_filter']==1,'Library navigation and active filter update underlying state')
    s.capture('library-playlists')
    s.click_label('Liked Songs');liked=s.capture('liked-songs');s.collection_geometry(liked)
    s.label(liked,'Playlist · 0 songs')
    s.check(liked['file']==shuffled['file'],'Liked Songs shares collection layout without changing playback')
    s.command('click',400,770)
    s.metrics()
    s.command('stop');s.command('start',3,'--card',card,'--preserve-config')
    resumed=s.snapshot()
    s.check(resumed['tab']==2 and resumed['library_filter']==1,'Library tab and filter survive restart')
    s.check(resumed['shuffle'] and resumed['priority']==6,'shuffle and manual queue survive restart')
    s.check(entries('Empty Playlist')==[str(card/'Music/track001.flac')],'playlist membership survives restart')

def preview_online(s,card):
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
    s.command('drag',240,65,240,610)
    # Offline provider metadata, parsed and rendered by production code.
    fixture=s.out/'online.json';fixture.write_text(json.dumps({'metadata':{'mediatype':'audio','title':'Night Lights','creator':'Night Ensemble','licenseurl':'https://creativecommons.org/licenses/by/4.0/'},'files':[{'name':'night-walk.flac','title':'Night Walk','size':'10485760','md5':'0123456789abcdef0123456789abcdef'}]}))
    queue_file=card/'Downloads/.sonix-downloads.json'
    queued_before=json.loads(queue_file.read_text()) if queue_file.exists() else None
    before=s.snapshot();s.setup('online-fixture '+str(fixture));online=s.capture('online-track-details')
    s.label(online,'Download to microSD');s.label(online,'More by this artist')
    s.check(online['modal'] and online['file']==before['file'],'online track details preserve playback')
    queued_after=json.loads(queue_file.read_text()) if queue_file.exists() else None
    s.check(queued_before==queued_after,'opening track details does not enqueue a download')
    s.click_label('View downloads');s.check(not s.snapshot()['modal'],'downloads action closes details sheet')
    s.capture('online-downloads');s.metrics()


def main():
    p=argparse.ArgumentParser();p.add_argument('repo',type=Path);p.add_argument('--baseline',type=Path);p.add_argument('--card',type=Path)
    p.add_argument('--journey',choices=['collections','empty-liked','preview-online'],default='collections')
    a=p.parse_args();repo=a.repo.resolve();out=Path(tempfile.mkdtemp(prefix='ui-collections-',dir=repo/'artifacts'))
    s=Session(repo,out,a.baseline);started=False
    try:
        card=a.card.resolve() if a.card else create(repo,out/'sd')
        s.command('start',3,'--card',card);started=True
        if a.journey=='collections':journey(s,card)
        elif a.journey=='preview-online':preview_online(s,card)
        else:
            s.command('click',400,770);s.click_label('Liked Songs')
            state=s.capture('liked-songs');_,play=s.collection_geometry(state)
            s.label(state,'Playlist · 0 songs')
            s.command('click',119,(play[1]+play[3])//2)
            after=s.snapshot()
            s.check(after['file']==state['file'] and after['priority']==state['priority'],'empty liked collection cannot enqueue a nonexistent song')
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
