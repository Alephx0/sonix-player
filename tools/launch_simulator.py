#!/usr/bin/env python3
"""Launch the native interactive player on WSLg, using a private simulated card."""
import argparse
import configparser
import os
from pathlib import Path
import shutil
import signal
import subprocess
import time

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('repo', type=Path)
p.add_argument('--card-root', type=Path)
p.add_argument('--restart', action='store_true')
a = p.parse_args()
repo = a.repo.resolve()
player = repo / 'sonix-player'
binary = player / 'sonix_player_host'
out = repo / 'artifacts/interactive'
out.mkdir(parents=True, exist_ok=True)
pid_file = out / 'pid'
if not binary.is_file():
    raise SystemExit('Build the host executable first with tools/build_wsl.py.')
if pid_file.exists():
    pid = int(pid_file.read_text())
    cmdline = Path(f'/proc/{pid}/cmdline')
    if cmdline.exists() and str(binary).encode() in cmdline.read_bytes().split(b'\0'):
        if not a.restart:
            raise SystemExit('This simulator is already running; use --restart to reload the build.')
        os.kill(pid, signal.SIGINT)
        for _ in range(100):
            if not cmdline.exists() or not cmdline.read_bytes():
                break
            time.sleep(.1)
        else:
            raise SystemExit('The previous simulator is still shutting down; try again shortly.')
card = a.card_root.resolve() if a.card_root else out / 'sd'
card.mkdir(parents=True, exist_ok=True)
config_path = out / 'device_config.ini'
config = configparser.ConfigParser(interpolation=None)
config.read(config_path)
for section, values in {
    'power': {'screen_off_seconds': '0', 'standby_mem': '0'},
    'screen': {'player_layout_alt': '3'},
    'player': {'remember_track': '1'},
    'clock': {'configured': '1'},
}.items():
    if not config.has_section(section):
        config.add_section(section)
    config[section].update(values)
with config_path.open('w') as stream:
    config.write(stream)
shutil.copytree(repo / 'sonix-packer/assets/R1/usr/resource', player / 'usr/resource', dirs_exist_ok=True)
alsa = out / 'alsa.conf'
alsa.write_text('pcm.!default { type pulse }\nctl.!default { type pulse }\n')
env = dict(os.environ, SONIX_PANEL='480x800', SONIX_SD_ROOT=str(card),
           SONIX_CONFIG=str(config_path), SONIX_EBOOK_CONFIG=str(out / 'ebook.ini'),
           SONIX_NO_MOUNT='1', SONIX_NO_SYSSERVER='1',
           SDL_VIDEODRIVER='x11', SDL_RENDER_DRIVER='software', ALSA_CONFIG_PATH=str(alsa))
with (out / 'simulator.log').open('w') as log:
    app = subprocess.Popen([str(binary)], cwd=player, env=env, stdin=subprocess.DEVNULL,
                           stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
pid_file.write_text(str(app.pid))
for _ in range(100):
    if app.poll() is not None:
        raise SystemExit('Simulator exited; inspect artifacts/interactive/simulator.log.')
    found = subprocess.run(['xdotool', 'search', '--onlyvisible', '--pid', str(app.pid)],
                           env=env, capture_output=True, text=True)
    if found.returncode == 0:
        window = found.stdout.splitlines()[0]
        (out / 'window').write_text(window)
        subprocess.run(['xdotool', 'windowactivate', window], env=env, capture_output=True)
        print(f'Interactive simulator running (PID {app.pid}). Card: {card}')
        break
    time.sleep(.1)
else:
    raise SystemExit('No visible window yet; inspect artifacts/interactive/simulator.log.')
