#!/usr/bin/env python3
"""Run the real SDL simulator in Xvfb and capture reproducible evidence.

Run on Linux: python3 tools/host_smoke.py /path/to/repo --label baseline
No device is accessed. A private SD root and configuration are used.
"""
import argparse
import json
import os
from pathlib import Path
import shutil
import signal
import secrets
import configparser
import hashlib
import sqlite3
import subprocess
import time


def run(repo, label, online=False, layout=None, card_root=None, cycles=0, download=False, browse=False, paging=False):
    repo = Path(repo).resolve()
    player = repo / 'sonix-player'
    out = repo / 'artifacts' / label
    out.mkdir(parents=True, exist_ok=True)
    card = Path(card_root).resolve() if card_root else out / 'sd'
    card.mkdir(exist_ok=True)
    if download and (card / 'Downloads/.sonix-downloads.json').exists():
        raise RuntimeError('Use a fresh --label/card root for the touch download test')
    config = out / 'device_config.ini'
    config.write_text('[ui]\nlanguage=English\ntheme=dark\n[clock]\nconfigured=1\n'
                      '[power]\nscreen_off=0\nstandby=0\n')
    if layout is not None:
        with config.open('a') as stream:
            stream.write(f'[screen]\nplayer_layout_alt={layout}\n')
    shutil.copytree(repo / 'sonix-packer/assets/R1/usr/resource',
                    player / 'usr/resource', dirs_exist_ok=True)
    env = dict(os.environ, SONIX_PANEL='480x800', SONIX_SD_ROOT=str(card),
               SONIX_CONFIG=str(config), SONIX_EBOOK_CONFIG=str(out / 'ebook.ini'),
               SONIX_NO_MOUNT='1', SONIX_NO_SYSSERVER='1', SDL_AUDIODRIVER='dummy',
               SDL_VIDEODRIVER='x11', SDL_RENDER_DRIVER='software')
    # Read display readiness from Xvfb; display :99 must be available.
    rd, wr = os.pipe()
    # WSLg owns /tmp/.X11-unix read-only. Use an authenticated TCP display
    # for this short-lived isolated test instead of altering WSLg's sockets.
    auth = out / 'Xauthority'
    auth.touch(mode=0o600, exist_ok=True)
    cookie = secrets.token_hex(16)
    subprocess.run(['xauth', '-f', str(auth), 'add', ':99', '.', cookie], check=True)
    xvfb = subprocess.Popen(['Xvfb', ':99', '-displayfd', str(wr), '-screen', '0',
                             '480x800x24', '-nolisten', 'unix', '-nolisten', 'local',
                             '-listen', 'tcp', '-auth', str(auth)], pass_fds=(wr,),
                            stdout=subprocess.DEVNULL, stderr=(out / 'xvfb.log').open('w'))
    os.close(wr)
    with os.fdopen(rd) as stream:
        number = stream.readline().strip()
        if not number:
            xvfb.wait(timeout=5)
            raise RuntimeError('Xvfb could not start; see xvfb.log')
        env['DISPLAY'] = 'localhost:' + number
    env['XAUTHORITY'] = str(auth)
    print('Xvfb display:', env['DISPLAY'], flush=True)
    subprocess.run(['xdotool', 'getdisplaygeometry'], env=env, check=True)
    process = None
    metrics = {'kind': 'host simulator, not device performance', 'resolution': [480, 800]}
    try:
        with (out / 'simulator.log').open('w') as log:
            started = time.monotonic()
            process = subprocess.Popen([str(player / 'sonix_player_host')], cwd=player,
                                       env=env, stdout=log, stderr=subprocess.STDOUT)
            window = ''
            for _ in range(200):
                if process.poll() is not None:
                    raise RuntimeError(f'simulator exited: {process.returncode}')
                found = subprocess.run(['xdotool', 'search', '--onlyvisible', '--pid', str(process.pid)],
                                       env=env, capture_output=True, text=True)
                if found.returncode == 0:
                    window = found.stdout.splitlines()[0]
                    break
                time.sleep(.1)
            if not window:
                raise RuntimeError('no visible SDL window after 20 seconds')
            metrics['window_visible_seconds'] = round(time.monotonic() - started, 3)
            time.sleep(2)
            def shot(name):
                subprocess.run(['import', '-window', window, str(out / (name + '.png'))],
                               env=env, check=True, timeout=15)
            shot('home')
            if label != 'baseline':
                def click(x, y, delay=.8):
                    subprocess.run(['xdotool', 'mousemove', '--window', window, str(x), str(y),
                                    'click', '1'], env=env, check=True)
                    time.sleep(delay)
                for name, x in [('search', 240), ('library', 400), ('home-return', 80)]:
                    click(x, 760)
                    shot(name)
                click(240, 760)
                click(410, 208)
                shot('provider-settings')
                click(240, 250)
                shot('provider-disabled')
                click(240, 250)
                click(240, 320)  # Prefer MP3, without hiding other formats.
                saved = configparser.ConfigParser()
                saved.read(config)
                assert saved.getboolean('discovery', 'enabled')
                assert saved.getint('discovery', 'format') == 1
                metrics['provider_preferences_saved'] = True
                click(240, 760)
                if online:
                    click(315, 208)  # Online only: first result is a release.
                click(160, 145)
                shot('keyboard')
                if online:
                    # Exercise the actual touch keyboard, HTTPS provider and
                    # release navigation. This opt-in contacts archive.org.
                    for x, y in [(290, 608), (31, 534), (190, 608), (291, 534)]:
                        click(x, y)  # bach
                    click(430, 683)
                    time.sleep(15)
                    shot('online-results')
                    if paging:
                        def bottom():
                            # LVGL's SDL pointer input follows touch drags; wheel
                            # events are not mapped to this list.
                            for _ in range(6):
                                drag = ['xdotool', 'mousemove', '--window', window, '440', '600', 'mousedown', '1']
                                for y in range(580, 279, -20):
                                    drag += ['mousemove', '--window', window, '440', str(y), 'sleep', '.03']
                                drag += ['mouseup', '1']
                                subprocess.run(drag, env=env, check=True)
                                time.sleep(.4)
                            time.sleep(2)
                        bottom()
                        shot('page-one-footer')
                        click(110, 608)
                        time.sleep(15)
                        shot('page-two')
                        bottom()
                        shot('page-two-footer')
                        click(110, 608)
                        time.sleep(15)
                        shot('page-one-return')
                    click(200, 340)
                    time.sleep(15)
                    shot('online-release')
                    if browse:
                        click(250, 290)  # Artist releases, using source creator.
                        time.sleep(15)
                        shot('artist-releases')
                        # Return to the original text query, then reopen release.
                        click(160, 145)
                        click(430, 683)
                        time.sleep(15)
                        click(200, 340)
                        time.sleep(15)
                    if download:
                        click(200, 400)  # First source file, after release navigation row.
                        shot('touch-transfer-start')
                        deadline = time.monotonic() + 120
                        manifest = card / 'Downloads/.sonix-downloads.json'
                        job = None
                        while time.monotonic() < deadline:
                            if process.poll() is not None:
                                raise RuntimeError('Simulator exited during transfer')
                            if manifest.exists():
                                document = json.loads(manifest.read_text())
                                job = next(iter(document['jobs']), None)
                                if job and job['state'] == 4:
                                    raise RuntimeError('Touch-initiated transfer failed; inspect simulator and screenshot')
                                if job and job['state'] == 3:
                                    break
                            time.sleep(.25)
                        if not job or job['state'] != 3:
                            raise RuntimeError('No completed touch-initiated transfer before deadline')
                        item = job['item']
                        sidecars = list((card / 'Downloads').glob('*.source.json'))
                        sidecar = next(p for p in sidecars if json.loads(p.read_text())['md5'] == item['md5'])
                        audio = Path(str(sidecar).removesuffix('.source.json'))
                        assert audio.stat().st_size == item['bytes']
                        with audio.open('rb') as stream:
                            assert hashlib.file_digest(stream, 'md5').hexdigest() == item['md5']
                        db_path = card / '.local/library.db'
                        indexed = False
                        for _ in range(40):
                            if db_path.exists():
                                with sqlite3.connect(f'file:{db_path}?mode=ro', uri=True) as db:
                                    indexed = db.execute('SELECT name FROM MEDIA_TABLE WHERE path=?', (str(audio),)).fetchone()
                                if indexed:
                                    break
                            time.sleep(.25)
                        assert indexed, 'Completed UI download was not automatically indexed'
                        metrics['touch_download'] = {'source': item['id'], 'bytes': item['bytes'],
                            'md5_verified': True, 'automatically_indexed': True}
                        shot('touch-transfer-indexed')
                for _ in range(cycles):
                    for x in [80, 240, 400]:
                        click(x, 760, .15)
                metrics['navigation_cycles'] = cycles
                click(400, 760)
                click(140, 375)
                shot('downloads')
                # A fixture from test_live_download.py, if present, was restored
                # paused. Validate its existing bytes, then open Now Playing.
                if not download and list((card / 'Downloads').glob('*.mp3')):
                    click(140, 275)
                    time.sleep(2)
                    shot('download-verified')
                    click(140, 275)
                    time.sleep(1)
                    shot('now-playing')
            status = Path(f'/proc/{process.pid}/status').read_text()
            for line in status.splitlines():
                if line.startswith(('VmRSS:', 'VmHWM:', 'VmSize:', 'Threads:')):
                    key, val = line.split(':', 1)
                    metrics[key] = val.strip()
            metrics['alive_after_navigation'] = process.poll() is None
    finally:
        if process and process.poll() is None:
            process.send_signal(signal.SIGINT)
            try:
                process.wait(timeout=20)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
        xvfb.terminate()
        xvfb.wait(timeout=10)
        (out / 'metrics.json').write_text(json.dumps(metrics, indent=2) + '\n')
    print(json.dumps(metrics, indent=2))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('repo')
    parser.add_argument('--label', default='integrated')
    parser.add_argument('--online', action='store_true', help='Exercise live catalog search')
    parser.add_argument('--layout', type=int, choices=range(4), help='Exercise a saved Now Playing layout')
    parser.add_argument('--card-root', type=Path, help='Reuse a fixture SD root (run sequentially)')
    parser.add_argument('--cycles', type=int, default=0, help='Repeated Home/Search/Library cycles')
    parser.add_argument('--download', action='store_true', help='Download first live result by touch; requires fresh card')
    parser.add_argument('--browse', action='store_true', help='Visit artist releases in the live catalog scenario')
    parser.add_argument('--paging', action='store_true', help='Visit next/previous catalog pages by touch')
    args = parser.parse_args()
    run(args.repo, args.label, args.online or args.download or args.browse or args.paging, args.layout, args.card_root, args.cycles, args.download, args.browse, args.paging)
