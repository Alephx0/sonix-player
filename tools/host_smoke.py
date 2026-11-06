#!/usr/bin/env python3
"""Run the native local player smoke journey; online smoke modes were retired."""
import argparse
from pathlib import Path
import subprocess
import sys
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('repo',type=Path)
p.add_argument('--journey',choices=['collections','preview-local','local-search'],default='preview-local')
a=p.parse_args()
subprocess.run([sys.executable,str(Path(__file__).with_name('test_ui_collections.py')),str(a.repo),'--journey',a.journey],check=True)
