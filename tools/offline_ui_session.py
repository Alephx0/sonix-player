#!/usr/bin/env python3
"""Start/drive/stop a private native SDL session for offline UI evidence."""
import argparse
import json
import os
from pathlib import Path
import secrets
import signal
import subprocess
import time

p=argparse.ArgumentParser()
p.add_argument('repo',type=Path)
p.add_argument('action',choices=['start','click','drag','shot','stop'])
p.add_argument('args',nargs='*')
a=p.parse_args()
repo=a.repo.resolve();out=repo/'artifacts/offline-ui';out.mkdir(parents=True,exist_ok=True)
state_file=out/'session.json'
if a.action=='start':
    if state_file.exists():raise SystemExit('Stop the previous session first')
    auth=out/'Xauthority';auth.touch(mode=0o600,exist_ok=True)
    subprocess.run(['xauth','-f',str(auth),'add',':99','.',secrets.token_hex(16)],check=True)
    rd,wr=os.pipe()
    xvfb=subprocess.Popen(['Xvfb',':99','-displayfd',str(wr),'-screen','0','480x800x24','-nolisten','unix','-nolisten','local','-listen','tcp','-auth',str(auth)],pass_fds=(wr,),stdin=subprocess.DEVNULL,stdout=subprocess.DEVNULL,stderr=(out/'xvfb.log').open('w'),start_new_session=True)
    os.close(wr)
    with os.fdopen(rd) as stream:display=stream.readline().strip()
    if not display:raise SystemExit('Xvfb failed')
    layout=int(a.args[0]) if a.args else 3
    if layout not in range(4):raise SystemExit('Layout must be 0, 1, 2 or 3')
    config=out/'device_config.ini'
    config.write_text(f'[ui]\nlanguage=English\ntheme=dark\n[clock]\nconfigured=1\n[power]\nscreen_off_seconds=0\nstandby_mem=0\n[screen]\nplayer_layout_alt={layout}\n[player]\nremember_track=1\n')
    env=dict(os.environ,DISPLAY='localhost:'+display,XAUTHORITY=str(auth),SONIX_PANEL='480x800',SONIX_SD_ROOT=str(repo/'artifacts/offline-fixture/sd'),SONIX_CONFIG=str(config),SONIX_EBOOK_CONFIG=str(out/'ebook.ini'),SONIX_NO_MOUNT='1',SONIX_NO_SYSSERVER='1',SDL_VIDEODRIVER='x11',SDL_RENDER_DRIVER='software')
    alsa=out/'alsa.conf';alsa.write_text('pcm.!default { type pulse }\nctl.!default { type pulse }\n')
    env['ALSA_CONFIG_PATH']=str(alsa)
    app=subprocess.Popen([str(repo/'sonix-player/sonix_player_host')],cwd=repo/'sonix-player',env=env,stdin=subprocess.DEVNULL,stdout=(out/'simulator.log').open('w'),stderr=subprocess.STDOUT,start_new_session=True)
    window=''
    for _ in range(100):
        if app.poll() is not None:raise SystemExit('Simulator exited')
        found=subprocess.run(['xdotool','search','--onlyvisible','--pid',str(app.pid)],env=env,capture_output=True,text=True)
        if found.returncode==0:window=found.stdout.splitlines()[0];break
        time.sleep(.1)
    if not window:raise SystemExit('No visible SDL window')
    state={'pid':app.pid,'xvfb':xvfb.pid,'window':window,'display':env['DISPLAY'],'auth':str(auth)}
    state_file.write_text(json.dumps(state));time.sleep(3)
else:
    state=json.loads(state_file.read_text())
    env=dict(os.environ,DISPLAY=state['display'],XAUTHORITY=state['auth'])
window=state['window']
if a.action=='click':
    subprocess.run(['xdotool','mousemove','--window',window,*a.args[:2],'click','1'],env=env,check=True);time.sleep(.8)
elif a.action=='drag':
    x,y,ex,ey=map(int,a.args[:4]);cmd=['xdotool','mousemove','--window',window,str(x),str(y),'mousedown','1','sleep',a.args[4] if len(a.args)>4 else '.15']
    for i in range(1,21):cmd+=['mousemove','--window',window,str(x+(ex-x)*i//20),str(y+(ey-y)*i//20),'sleep','.03']
    cmd+=['mouseup','1'];subprocess.run(cmd,env=env,check=True);time.sleep(.8)
elif a.action=='stop':
    for key,expected in [('pid',b'sonix_player_host'),('xvfb',b'Xvfb')]:
        pid=state[key];proc=Path(f'/proc/{pid}/cmdline')
        if proc.exists() and expected in proc.read_bytes():os.kill(pid,signal.SIGINT if key=='pid' else signal.SIGTERM)
    state_file.unlink()
if a.action!='stop':
    name=a.args[0] if a.action=='shot' and a.args else a.action
    subprocess.run(['import','-window',window,str(out/(name+'.png'))],env=env,check=True)
    print(out/(name+'.png'))
