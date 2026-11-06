#!/usr/bin/env python3
"""Real SDL navigation and release-type regression, including Large touch bounds.

Host timings are diagnostic only, not R1 latency measurements. An optional card
must be an isolated test fixture (the test changes its release classification).
"""
import argparse
import json
from pathlib import Path
import sqlite3
import tempfile
import time
from test_ui_collections import Session, create
from test_ui_device_fixes import seek_label


def journey(s, card):
    def texts():
        return [n.get('text', '') for n in s.visible(s.snapshot())]

    def artist():
        s.command('click', 240, 770)
        s.setup('search-text Night Ensemble')
        s.click_label('All')
        s.click_label('Artists')
        state = s.snapshot()
        row = next(n for n in s.visible(state) if n.get('text') == 'Night Ensemble'
                   and 170 < n['rect'][1] < 500)
        s.click_node(row)
        time.sleep(.5)

    def options():
        node = s.label(s.snapshot(), 'Night Lights')
        x, y, r, b = node['rect']
        cx, cy = round((x+r)/2*s.zoom), round((y+b)/2*s.zoom)
        s.command('drag', cx, cy, cx+1, cy, 1.1)
        s.check(s.snapshot()['modal'], 'release long press opens a bottom sheet')
        s.label(s.snapshot(), 'Release type')

    s.setup('playlist Night Drive')
    # At Large, the first song starts below the persistent footer. Bring it
    # into the list viewport before tapping its label.
    s.click_node(seek_label(s, 'Night Walk'))
    before = s.snapshot()
    s.check(before['status'] == 1, 'fixture playback starts')
    for tab, x in [(0, 80), (1, 240), (0, 80), (2, 400), (1, 240), (0, 80)]:
        s.command('click', x, 770)
        state = s.snapshot()
        s.check(state['route'] == 'main' and state['tab'] == tab,
                'tab tap reaches its destination: %d' % tab)
        s.check(state['file'] == before['file'] and state['status'] == 1,
                'tab navigation preserves playback: %d' % tab)
    s.capture('responsive-home-large')
    artist()
    s.click_label('Singles & EPs')
    s.label(s.snapshot(), 'Night Lights')
    s.check(any('Single (estimated)' in t for t in texts()),
            'three-track untagged release appears in Singles & EPs')
    options()
    sheet = s.capture('release-type-sheet-large')
    for name in ('Automatic', 'Album', 'Single', 'EP'):
        node = s.label(sheet, name)
        s.check(0 <= node['rect'][0] < node['rect'][2] <= 384
                and 0 <= node['rect'][1] < node['rect'][3] <= 640,
                'release type choice fits Large display: ' + name)
    s.click_label('Album')
    time.sleep(.5)
    s.check(not s.snapshot()['modal'], 'selecting a type dismisses the sheet')
    s.check('Night Lights' not in texts(), 'manual Album leaves Singles & EPs immediately')
    s.click_label('Albums')
    s.label(s.snapshot(), 'Night Lights')
    s.capture('release-type-album-large')
    s.check(s.snapshot()['file'] == before['file'] and s.snapshot()['status'] == 1,
            'changing release type preserves playback')
    with sqlite3.connect(card/'.local/library.db') as db:
        s.check(db.execute('SELECT type FROM RELEASE_TYPE_OVERRIDE').fetchone() == ('Album',),
                'type override persists in the library database')
    # Reopen through the actual Search/artist journey, not a direct screen jump.
    artist()
    s.click_label('Albums')
    s.label(s.snapshot(), 'Night Lights')
    options()
    s.click_label('Automatic')
    s.click_label('Singles & EPs')
    s.label(s.snapshot(), 'Night Lights')
    s.capture('release-type-restored-large')
    with sqlite3.connect(card/'.local/library.db') as db:
        s.check(db.execute('SELECT COUNT(*) FROM RELEASE_TYPE_OVERRIDE').fetchone()[0] == 0,
                'Automatic removes the local correction and restores track-count estimate')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('repo', type=Path)
    parser.add_argument('--card', type=Path)
    args = parser.parse_args()
    repo = args.repo.resolve()
    out = Path(tempfile.mkdtemp(prefix='ui-responsiveness-', dir=repo/'artifacts'))
    s = Session(repo, out)
    s.env['SONIX_ZOOM'] = '125'
    s.zoom = 1.25
    started = False
    try:
        card = args.card.resolve() if args.card else create(repo, out/'sd')
        with sqlite3.connect(card/'.local/library.db') as db:
            db.execute("UPDATE RELEASE_META SET type='' WHERE path IN "
                       "(SELECT path FROM MEDIA_TABLE WHERE album='Night Lights')")
            db.execute('DELETE FROM RELEASE_TYPE_OVERRIDE')
        s.command('start', 3, '--card', card)
        started = True
        time.sleep(2)
        journey(s, card)
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
