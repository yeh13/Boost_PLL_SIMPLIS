"""Compile-only controlled include-path test for Icarus 10.1.1."""
import json
import subprocess
from pathlib import Path
from step5c_preflight import HERE,IV,env,short
root=HERE/'interface_audit/20261001T074233_838157Z/path_probe'
root.mkdir(exist_ok=False)
leaf=root/'leaf.v';leaf.write_text('module leaf; endmodule\n')
results=[]
for name,path in [('relative','leaf.v'),('absolute_forward',leaf.as_posix()),
                  ('absolute_short_forward',short(leaf)),('absolute_backslash',str(leaf))]:
    top=root/(name+'.v');top.write_text('`include "'+path+'"\n')
    p=subprocess.run([str(IV),'-I',str(root),'-s','leaf','-o',str(root/(name+'.vvp')),str(top)],env=env(),cwd=root,capture_output=True,text=True)
    (root/(name+'.log')).write_text(p.stdout+p.stderr)
    results.append(dict(case=name,include=path,return_code=p.returncode,log=p.stdout+p.stderr))
(root/'results.json').write_text(json.dumps(results,indent=2))
print(json.dumps(results,indent=2))
