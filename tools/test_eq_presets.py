"""Exercise native preset application and a fresh-process custom preset load."""
from pathlib import Path
import subprocess
import sys
import tempfile

repo=Path(sys.argv[1]).resolve()
player=repo/'sonix-player'
with tempfile.TemporaryDirectory(prefix='sonix-eq-') as tmp:
    work=Path(tmp)
    subprocess.run(['gcc','-DHOST_BUILD=1','-I'+str(player),
        str(player/'tests/eq_presets.c'),str(player/'src/system/audio/eq.c'),
        str(player/'src/system/core/config.c'),'-lm','-lpthread','-o',str(work/'test')],check=True)
    for phase in ('save','reload'):
        subprocess.run([str(work/'test'),str(work),phase],check=True)
