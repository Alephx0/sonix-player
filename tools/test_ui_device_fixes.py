#!/usr/bin/env python3
"""Native pointer regressions for the R1 repair batch; no writes to real cards."""
import json
import configparser
import os
import re
from pathlib import Path
import sqlite3
import sys
import tempfile
import time

from test_ui_collections import Session, create


def on_screen(s, st):
    """Exclude children clipped by a scrolled list, not just hidden flags."""
    by_id = {n['id']:n for n in st['nodes']}
    result = []
    for n in s.visible(st):
        l,t,r,b = n['rect']
        r,b = min(r,480/s.zoom),min(b,800/s.zoom)
        parent = n['parent']
        while parent >= 0:
            p = by_id[parent]
            if p['scrollable']:
                pl,pt,pr,pb = p['rect']
                l,t,r,b = max(l,pl),max(t,pt),min(r,pr),min(b,pb)
            parent = p['parent']
        if r>l and b>t:
            result.append(n)
    return result


def seek_label(s, text, attempts=8):
    for _ in range(attempts):
        st = s.snapshot()
        matches = [n for n in on_screen(s,st) if n.get('text','').startswith(text)
                   and n['rect'][1]*s.zoom>120 and n['rect'][3]*s.zoom<(785 if st['modal'] else 640)]
        if matches:
            exact=[n for n in matches if n.get('text')==text]
            return (exact or matches)[-1]
        s.command('drag',240,570,240,300,.3)
        s.settled()
    raise AssertionError('Could not scroll to '+text)


def player_repairs(s, card):
    s.setup('playlist Night Drive')
    s.click_node(seek_label(s,'Night Walk'))
    time.sleep(.5)
    st=s.snapshot()
    s.check(st['status']==1 and not st['now_playing_open'], 'track tap starts preview playback')
    # Repeated list flicks are never an instruction to expand the player.
    for x in (100,240,430):
        s.command('drag',x,540,x+8,250,.3)
        s.check(not s.snapshot()['now_playing_open'], 'scroll away from mini-player stays in collection')
    s.command('click',150,690)
    time.sleep(.7)
    st=s.capture('now-playing-large')
    s.check(st['now_playing_open'], 'mini-player tap still opens Now Playing')
    vis=on_screen(s,st)
    fmt=next(n for n in vis if re.fullmatch(r'\d+/[\d.]+ FLAC',n.get('text','')))
    par=next(n for n in st['nodes'] if n['id']==fmt['parent'])
    s.check(abs(fmt['rect'][0]+fmt['rect'][2]-par['rect'][0]-par['rect'][2])<=2
            and abs(fmt['rect'][1]+fmt['rect'][3]-par['rect'][1]-par['rect'][3])<=2,
            'format text centered horizontally and vertically inside badge')
    dots=max((n for n in vis if n['clickable'] and n['rect'][1]*s.zoom<120
              and n['rect'][2]-n['rect'][0]<=64), key=lambda n:n['rect'][0])
    s.check(480-dots['rect'][2]*s.zoom<=12, 'Now Playing dots sit at screen right corner')
    art=max((n for n in vis if n['type']=='image'),key=lambda n:n['rect'][2]-n['rect'][0])
    s.click_node(art);time.sleep(.8)
    focused=s.capture('focused-artwork')
    big=max((n for n in on_screen(s,focused) if n['type']=='image'),key=lambda n:n['rect'][2]-n['rect'][0])
    s.check((big['rect'][2]-big['rect'][0])*s.zoom>=440, 'focused artwork fills almost the full screen width')
    s.command('click',80,100);time.sleep(.3)
    s.check(s.snapshot()['node_count']<focused['node_count'], 'outside tap closes focused art')
    s.click_node(art);time.sleep(.4)
    s.command('click',440,45);time.sleep(.3)
    s.check(s.snapshot()['node_count']<focused['node_count'], 'X closes focused art')
    slider=next(n for n in vis if n['type']=='slider' and n['rect'][2]-n['rect'][0]>200)
    x,y,r,b=slider['rect'];py=round((y+b)/2*s.zoom)
    s.setup('seek 0')
    s.command('drag',round(x*s.zoom),py,240,py,.5)
    s.check(s.snapshot()['seconds']>10, 'seek can be grabbed at start of progress line')
    s.setup('seek 172')
    s.command('drag',round(r*s.zoom)-1,py,240,py,.04)
    s.check(60<s.snapshot()['seconds']<120, 'seek can be grabbed at right end of progress line')
    s.setup('seek 2')
    # The actions row has Lyrics at left and Sound / Queue at right.
    bottom=[n for n in on_screen(s,s.snapshot()) if n['clickable']
            and 35<=n['rect'][2]-n['rect'][0]<=56 and n['rect'][1]*s.zoom>700]
    queue=max(bottom,key=lambda n:n['rect'][0])
    s.click_node(queue);time.sleep(.5)
    q=s.capture('queue-spacing')
    heading=next(n for n in on_screen(s,q) if n.get('text','').startswith('Next from'))
    upcoming=next(n for n in on_screen(s,q) if n.get('text','').startswith('Caf') and n['id']>heading['id'])
    qimages=[n for n in on_screen(s,q) if n['type']=='image' and n['parent']==upcoming['parent']
             and n['rect'][2]-n['rect'][0]>=40]
    s.check(qimages and min(n['rect'][1] for n in qimages)>=heading['rect'][3]+4,
            'queue artwork starts below Next from heading')
    s.command('click',445,80);time.sleep(.3)
    s.check(not s.snapshot()['modal'], 'queue close control dismisses')
    sound=sorted(bottom,key=lambda n:n['rect'][0])[-2]
    s.click_node(sound);time.sleep(.4)
    s.label(s.snapshot(),'Sound')
    s.click_node(seek_label(s,'All audio settings'))
    time.sleep(.6)
    st=s.capture('all-audio-settings')
    s.check(not st['modal'] and not st['now_playing_open'], 'Sound sheet scroll reaches All audio settings')
    s.check(st['status']==1, 'audio continues through focus, queue and sound settings')
    s.command('click',54,108);time.sleep(.6)
    s.click_node(dots);time.sleep(.4)
    s.click_node(seek_label(s,'Track information'))
    time.sleep(.5)
    s.label(s.capture('track-details-accessible'),'Details')
    s.command('click',54,108);time.sleep(.5)
    s.command('drag',240,65,240,650,.4);time.sleep(.5)


def settings_repairs(s, card):
    s.command('click',80,774);time.sleep(.4)
    s.command('click',48,96);time.sleep(.4)
    s.click_label('Settings');time.sleep(.5)
    s.capture('settings-profile')
    s.click_node(seek_label(s,'Device'))
    time.sleep(.5)
    s.click_node(seek_label(s,'System'))
    time.sleep(.4)
    s.click_node(seek_label(s,'About'))
    time.sleep(.4)
    s.capture('device-about')
    s.click_node(seek_label(s,'Name'))
    s.setup('field-text Pocket Player')
    s.click_label('Save');time.sleep(.3)
    s.check(any(n.get('text')=='Pocket Player' for n in s.visible(s.snapshot())), 'device name changes immediately')
    s.click_node(seek_label(s,'Picture'))
    time.sleep(.4)
    s.capture('profile-picture-picker')
    s.command('click',440,80);time.sleep(.4)
    s.command('click',54,108);time.sleep(.5)
    s.click_node(seek_label(s,'Update from SD card'))
    time.sleep(.4)
    s.capture('sd-update-no-image')
    s.check(any('.upt' in n.get('text','') or 'battery' in n.get('text','').lower() or 'simulator' in n.get('text','').lower()
                for n in s.visible(s.snapshot())), 'SD updater provides preflight feedback without flashing')
    # Dismiss the informational dialog only, never confirm an update.
    choices=[n for n in s.visible(s.snapshot()) if n.get('text') in ('OK','Close','Cancel')]
    if choices:s.click_node(choices[-1])
    s.command('click',54,108);time.sleep(.5)
    s.command('click',54,108);time.sleep(.5)
    before=s.capture('settings-return-scroll')
    s.check(any(n.get('text')=='Device' and 120<n['rect'][1]*s.zoom<640 for n in on_screen(s,before)),
            'return from nested Device pages preserves Settings scroll position')
    s.click_node(seek_label(s,'Device'));time.sleep(.4)
    s.click_node(seek_label(s,'Storage'))
    time.sleep(1.2)
    st=s.capture('storage-categories')
    s.check(any(n.get('text')=='Music' for n in on_screen(s,st)), 'Storage displays category usage')
    # All lower categories and internal storage remain reachable.
    seek_label(s,'Free')
    s.capture('storage-bottom')


def controls_repairs(s, card):
    def config():
        c=configparser.ConfigParser()
        c.read(s.repo/'artifacts/offline-ui/device_config.ini')
        return c
    s.command('click',80,774);s.command('click',48,96)
    s.click_label('Settings')
    s.click_node(seek_label(s,'Playback'))
    s.click_node(seek_label(s,'Now Playing'))
    s.click_node(seek_label(s,'Waveform'))
    s.check(config().getboolean('player','progress_wave'), 'Waveform control saves the progress preference')
    s.click_label('Line')
    s.check(not config().getboolean('player','progress_wave'), 'Line control restores the default progress style')
    s.capture('progress-style-options')
    s.command('click',54,108);s.command('click',54,108)
    s.click_node(seek_label(s,'Audio quality'))
    s.click_node(seek_label(s,'Equaliser'))
    eq=s.capture('equalizer-presets')
    rock=next(n for n in on_screen(s,eq) if n.get('text')=='Rock')
    s.click_node(rock)
    s.check(config().get('eq','preset')=='eq_rock', 'Rock preset is applied through the visible chip')
    # Selecting a preset can horizontally scroll its chip into view.
    cy=round((rock['rect'][1]+rock['rect'][3])/2*s.zoom)
    s.command('drag',100,cy,410,cy,.04)
    s.click_label('+ Save');time.sleep(.4)
    s.setup('field-text My listening')
    s.click_label('OK');time.sleep(.5)
    s.check((card/'.local/eq/My listening.ini').exists(), 'custom EQ preset saved through name keyboard')
    s.capture('equalizer-custom-saved')
    s.command('stop')
    s.command('start',3,'--card',card,'--preserve-config')
    time.sleep(.5)
    s.check(config().get('eq','preset')=='My listening', 'saved preset selection survives app restart')
    s.check(not config().getboolean('player','progress_wave'), 'Line preference survives app restart')


def library_gestures(s, card):
    def pins():
        with sqlite3.connect('file:' + str(card / '.local/collections.db') + '?mode=ro', uri=True) as db:
            return db.execute('SELECT kind,key FROM collection WHERE pin>0 ORDER BY kind,key').fetchall()

    with sqlite3.connect(card / '.local/collections.db') as db:
        s.check(db.execute("SELECT COUNT(*) FROM collection WHERE title LIKE 'Mix %'").fetchone()[0] == 120,
                '120 real indexed playlists exist; fixture exercises the scroll grip')
    s.command('click', 400, 774)
    time.sleep(.7)
    s.click_label('Playlists')
    time.sleep(.5)
    s.click_label('Recents')
    s.click_label('Alphabetical')
    time.sleep(.5)
    baseline = pins()
    s.command('drag', 240, 590, 240, 340, .25)
    time.sleep(.15)
    st = s.snapshot()
    grips = [n for n in s.visible(st) if n['clickable'] and n['rect'][2]-n['rect'][0] == 28
             and n['rect'][3]-n['rect'][1] == 56]
    s.check(len(grips) == 1, 'long library exposes exactly one fast-scroll grip')
    grip = grips[0]
    x,y,r,b = grip['rect']
    cx,cy = round((x+r)/2*s.zoom), round((y+b)/2*s.zoom)
    s.command('drag', cx, cy, cx, 570, .65)
    time.sleep(.6)
    after = s.capture('library-fast-scroll')
    s.check(pins() == baseline, 'fast-scroll grip never pins or unpins underlying rows')
    s.check(not after['now_playing_open'], 'library fast-scroll never opens Now Playing')
    names = [n.get('text','') for n in s.visible(after)]
    s.check(any(t.startswith('Mix ') and int(t[4:]) >= 70 for t in names),
            'fast-scroll actually jumps across the long library')
    s.command('click', 400, 774)
    time.sleep(.7)
    s.check(any(n.get('text','').startswith('A very long playlist') and 200 < n['rect'][1]*s.zoom < 400
                for n in s.visible(s.snapshot())),
            'reselecting Your Library returns directly to the top')
    for x in (440, 400, 280):
        # offline_ui_session's last argument is the initial hold, not speed.
        # A 400 ms hold intentionally opens a row's long-press action sheet.
        s.command('drag', x, 580, x-8, 320, .04)
        s.command('drag', x, 320, x+7, 580, .04)
    s.check(pins() == baseline, 'vertical scrolls with finger drift leave pins unchanged')
    s.check(not s.snapshot()['modal'], 'normal scrolling does not open a row action sheet')
    s.capture('library-vertical-scroll')
    s.command('click',400,774);s.settled()
    first=next(n for n in on_screen(s,s.snapshot()) if n.get('text','').startswith('A very long playlist'))
    cy=round((first['rect'][1]+first['rect'][3])/2*s.zoom)
    s.command('drag',145,cy,340,cy,.04)
    s.check(len(pins())==len(baseline)+1, 'intentional horizontal swipe still pins a collection')
    s.command('drag',145,cy,340,cy,.04)
    s.check(pins()==baseline, 'second intentional swipe unpins the same collection')


def main():
    repo = Path(sys.argv[1]).resolve()
    out = Path(tempfile.mkdtemp(prefix='ui-device-fixes-', dir=repo/'artifacts'))
    s = Session(repo, out)
    zoom = int(os.environ.get('Z', '125'))
    s.env['SONIX_ZOOM'] = str(zoom)
    s.zoom = zoom / 100
    started = False
    try:
        os.environ['SONIX_FIXTURE_PLAYLISTS'] = '120'
        card = create(repo, out/'sd')
        os.environ.pop('SONIX_FIXTURE_PLAYLISTS')
        s.command('start', 3, '--card', card)
        started = True
        time.sleep(2)
        only=sys.argv[2] if len(sys.argv)>2 else 'all'
        if only in ('all','library'):library_gestures(s, card)
        if only in ('all','player'):player_repairs(s, card)
        if only in ('all','settings'):settings_repairs(s, card)
        if only=='controls':controls_repairs(s, card)
        s.report['result'] = 'PASS'
    except Exception as exc:
        s.report.update(result='FAIL', error=repr(exc))
        if started:
            s.capture('failure')
        raise
    finally:
        if started:
            s.command('stop')
        (out/'report.json').write_text(json.dumps(s.report, indent=2))
        print('UI report:', out/'report.json', flush=True)


if __name__ == '__main__':
    main()
