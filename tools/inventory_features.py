from pathlib import Path
import re, subprocess
import argparse
root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser(description='Generate a baseline inventory without overwriting the reviewed feature ledger.')
parser.add_argument('--baseline', default='fe94fe796d93c6efa50d1107a734864c9c6a03d5')
parser.add_argument('--output', type=Path, default=root/'artifacts/upstream-inventory.md')
args=parser.parse_args()
sha=subprocess.check_output(['git','rev-parse',args.baseline],cwd=root,text=True).strip()
text=subprocess.check_output(['git','show',sha+':FEATURES.md'],cwd=root).decode('utf-8-sig')
rows=[]
section=''
for n,line in enumerate(text.splitlines(),1):
    if line.startswith('## '): section=line[3:]
    if line.startswith('- ') or (section=='Supported audio formats' and line.startswith('`.')):
        label=line[2:] if line.startswith('- ') else line
        label=label.replace('|','\\|')
        rows.append(f'| {section} | {label} | FEATURES.md:{n} | Preserved upstream source; regression verification pending |')
out='# Upstream compatibility inventory\n\nBaseline commit: `'+sha+'`. Generated before application changes. Each documented top-level feature is retained as its own row; descriptions include sub-capabilities. Preserved source is not proof of operational compatibility. Hardware-only behavior requires physical testing.\n\n| Area | Upstream capability | Evidence | Fork status |\n|---|---|---|---|\n'+'\n'.join(rows)+'\n'
args.output.parent.mkdir(parents=True, exist_ok=True)
args.output.write_text(out,encoding='utf-8')
print(f'Inventoried {len(rows)} documented feature entries at {sha}')
