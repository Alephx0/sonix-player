#!/usr/bin/env python3
"""Real LVGL waveform geometry and filtered Library grip regressions."""
from pathlib import Path
import json, sqlite3, sys, tempfile, time
from test_ui_collections import Session
from test_ui_device_fixes import on_screen, seek_label

repo=Path(sys.argv[1]).resolve(); card=Path(sys.argv[2]).resolve()
out=Path(tempfile.mkdtemp(prefix='ui-wave-library-',dir=repo/'artifacts'))
s=Session(repo,out); s.env['SONIX_ZOOM']=sys.argv[3] if len(sys.argv)>3 else '125'; s.zoom=int(s.env['SONIX_ZOOM'])/100

def set_wave(on):
    if s.snapshot()['now_playing_open']:
        s.command('drag',240,65,240,650,.1)
    s.command('click',80,774);s.command('click',48,96)
    s.click_label('Settings')
    s.click_node(seek_label(s,'Playback'))
    s.click_node(seek_label(s,'Now Playing'))
    s.click_node(seek_label(s,'Waveform' if on else 'Line'))
    for _ in range(5):
        if s.snapshot()['route']=='main':break
        s.command('click',54,108)
    s.command('click',150,690)
    time.sleep(.5)
    s.check(s.snapshot()['now_playing_open'],'return to Now Playing after progress preference change')

def geometry(name):
    st=s.capture(name);vis=on_screen(s,st)
    # The compact player remains mounted behind the full player. Its title
    # can be inside screen bounds but is occluded, so use the foreground row.
    title=max((n for n in vis if n.get('text')=='Night Walk'),key=lambda n:n['id'])
    slider=next(n for n in vis if n['type']=='slider' and n['rect'][2]-n['rect'][0]>200)
    s.check(title['rect'][3]<slider['rect'][1],name+': title stays above seek control')
    s.check(slider['rect'][3]*s.zoom<680,name+': seeking stays clear of utility controls')
    s.check(not st['modal'] and st['status']==1,name+': playback continues')
    # The menu itself must not acquire scrolling to hide an overflowing row.
    by_id={n['id']:n for n in st['nodes']}
    p=by_id[slider['parent']]
    if p['rect'][3]-p['rect'][1]<=64:p=by_id[p['parent']]
    s.check(abs(p.get('scroll_y',0))<=1,name+': controls do not overflow vertically')
    return slider['rect']

started=False
try:
    s.command('start',3,'--card',card);started=True;time.sleep(2)
    s.setup('playlist Night Drive')
    s.click_node(seek_label(s,'Night Walk'))
    # The worker must populate its cache while the player is still collapsed.
    deadline=time.monotonic()+15
    cache=card/'.local/waveform.dat'
    while (not cache.exists() or cache.stat().st_size<72) and time.monotonic()<deadline:time.sleep(.1)
    s.check(cache.exists() and cache.stat().st_size>=72,'waveform prepared while mini-player is collapsed')
    s.command('click',150,690);time.sleep(.5)
    geometry('line-before')
    set_wave(True);first=geometry('wave-first')
    set_wave(False);geometry('line-restored')
    set_wave(True);again=geometry('wave-again')
    s.check(first==again,'repeated waveform toggle keeps identical bounds')
    x,y,r,b=again;py=round((y+b)/2*s.zoom)
    s.command('drag',round(x*s.zoom),py,240,py,.05)
    s.check(30<s.snapshot()['seconds']<150,'waveform remains a functional seek control')
    s.command('drag',240,65,240,650,.1)
    s.command('click',400,774)
    # Filter chips scroll horizontally on Large.
    s.command('drag',420,180,115,180,.1)
    s.click_label('Artists')
    s.click_label('Recents');s.click_label('Alphabetical')
    for i in range(3):
        s.command('drag',240,560,240,400,.05)
        st=s.capture('artists-grip-'+str(i))
        grips=[n for n in on_screen(s,st) if n['clickable'] and n['rect'][2]-n['rect'][0]==28
               and n['rect'][3]-n['rect'][1]==56]
        s.check(len(grips)==1,'Artists grip is reachable after scroll')
        g=grips[0];x,y,r,b=g['rect']
        s.check(y*s.zoom>210 and b*s.zoom<690,'grip stays between Library header and mini-player')
        cx=round((x+r)/2*s.zoom)
        s.command('drag',cx,round((y+b)/2*s.zoom),cx,610 if i%2==0 else 330,.1)
        s.check(not s.snapshot()['modal'],'grip drag does not open a collection menu')
    st=s.capture('artists-after-jumps')
    s.check(not st['now_playing_open'],'Library grip does not open player')
    with sqlite3.connect(card/'.local/collections.db') as db:
        s.check(db.execute('SELECT COUNT(*) FROM collection WHERE pin>0').fetchone()[0]==0,
                'grip never pins an underlying artist')
    s.report['result']='PASS'
except Exception as exc:
    s.report.update(result='FAIL',error=repr(exc))
    if started:s.capture('failure')
    raise
finally:
    if started:s.command('stop')
    (out/'report.json').write_text(json.dumps(s.report,indent=2))
    print('UI report:',out/'report.json',flush=True)
