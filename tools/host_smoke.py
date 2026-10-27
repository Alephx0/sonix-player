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
import subprocess
import time


def run(repo, label, online=False):
    repo = Path(repo).resolve()
    player = repo / 'sonix-player'
    out = repo / 'artifacts' / label
    out.mkdir(parents=True, exist_ok=True)
    card = out / 'sd'
    card.mkdir(exist_ok=True)
    config = out / 'device_config.ini'
    config.write_text('[ui]\nlanguage=English\ntheme=dark\n[clock]\nconfigured=1\n'
                      '[power]\nscreen_off=0\nstandby=0\n')
    shutil.copytree(repo / 'sonix-packer/assets/R1/usr/resource',
                    player / 'usr/resource', dirs_exist_ok=True)
    env = dict(os.environ, SONIX_PANEL='480x800', SONIX_SD_ROOT=str(card),
               SONIX_CONFIG=str(config), SONIX_EBOOK_CONFIG=str(out / 'ebook.ini'),
               SONIX_NO_MOUNT='1', SONIX_NO_SYSSERVER='1', SDL_AUDIODRIVER='dummy',
               SDL_VIDEODRIVER='x11', SDL_RENDER_DRIVER='software')
    # Xvfb -displayfd avoids colliding with an existing desktop/session.
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
                def click(x, y):
                    subprocess.run(['xdotool', 'mousemove', '--window', window, str(x), str(y),
                                    'click', '1'], env=env, check=True)
                    time.sleep(.8)
                for name, x in [('search', 240), ('library', 400), ('home-return', 80)]:
                    click(x, 760)
                    shot(name)
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
                    click(200, 340)
                    time.sleep(15)
                    shot('online-release')
                click(400, 760)
                click(140, 375)
                shot('downloads')
                # A fixture from test_live_download.py, if present, was restored
                # paused. Validate its existing bytes, then open Now Playing.
                if list((card / 'Downloads').glob('*.mp3')):
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
    args = parser.parse_args()
    run(args.repo, args.label, args.online)
