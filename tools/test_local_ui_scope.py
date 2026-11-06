#!/usr/bin/env python3
"""Guard the local firmware boundary in source and freshly built native binaries."""
import argparse
from pathlib import Path
import re
import subprocess
p=argparse.ArgumentParser();p.add_argument('repo',type=Path);a=p.parse_args()
root=a.repo.resolve();src=root/'sonix-player/src'
removed=['system/discovery','gui/online','system/streaming','gui/streaming']
for name in removed:
 assert not list((src/name).glob('*.[ch]')),name+' must stay removed'
for name in ['system/decode/growfile.c','system/net/hls.c','system/device/ota.c','system/remote/dlna.c','system/library/lrclib.c','gui/nowplaying/lyricsearch.c','gui/audio/peqautoeq.c']:
 assert not (src/name).exists(),name
# Exclude comments and strings, so historical explanation is not mistaken for executable code.
forbidden=re.compile(r'\b(?:discovery_\w+|spotiflac_\w+|spotbye_\w+|download_queue_\w+|transfer_worker_\w+|growfile_\w+|http_download_open|http_stream_open|device_state_add_prepare_cb|streamkeys_\w+|qobuz_\w+|tidal_\w+|lrclib_\w+)\s*\(')
for path in src.rglob('*'):
 if path.suffix not in ('.c','.h') or path.name.startswith('sqlite'):continue
 code=re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"', '',path.read_text(encoding='utf-8'))
 assert not forbidden.search(code),str(path)
for binary in ['sonix_player_host','sonix_player_debug']:
 path=root/'sonix-player'/binary
 if not path.exists():continue
 symbols=subprocess.check_output(['nm',str(path)],text=True,stderr=subprocess.DEVNULL)
 assert not re.search(r'\b(?:discovery_|spotiflac_|spotbye_|download_queue_|growfile_|http_download_open|streamkeys_|qobuz_|tidal_|lrclib_)\w*$',symbols,re.M),binary
print('PASS local UI boundary: removed source, no provider/transfer entry points, native symbols clean')
