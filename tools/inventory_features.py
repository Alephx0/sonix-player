from pathlib import Path
import re, subprocess
root=Path(__file__).resolve().parents[1]
sha=subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip()
text=(root/'FEATURES.md').read_text(encoding='utf-8')
rows=[]
section=''
for n,line in enumerate(text.splitlines(),1):
    if line.startswith('## '): section=line[3:]
    if line.startswith('- ') or (section=='Supported audio formats' and line.startswith('`.')):
        label=line[2:] if line.startswith('- ') else line
        label=label.replace('|','\\|')
        rows.append(f'| {section} | {label} | FEATURES.md:{n} | Preserved upstream source; regression verification pending |')
out='# Upstream compatibility inventory\n\nBaseline commit: `'+sha+'`. Generated before application changes. Each documented top-level feature is retained as its own row; descriptions include sub-capabilities. Preserved source is not proof of operational compatibility. Hardware-only behavior requires physical testing.\n\n| Area | Upstream capability | Evidence | Fork status |\n|---|---|---|---|\n'+'\n'.join(rows)+'\n'
(root/'docs/custom-firmware/feature-matrix.md').write_text(out,encoding='utf-8')
print(f'Inventoried {len(rows)} documented feature entries at {sha}')
