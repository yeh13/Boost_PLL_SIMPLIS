from pathlib import Path
import os,subprocess,json,hashlib
R=Path(__file__).resolve().parent;C=R/'VOLTAGE_SOGI_SYNC_FIXED60_NEWPI.X'
M=Path('C:/Program Files/Microchip/MPLABX/v6.35/gnuBins/GnuWin32/bin/make.exe')
env=dict(os.environ)
env['PATH']=str(M.parent)+os.pathsep+'C:/Program Files/Microchip/MPLABX/v6.35/mplab_platform/bin'+os.pathsep+env['PATH']
cmd=[str(M),'-f','nbproject/Makefile-default.mk','OBJECTDIR=build_make_check','DISTDIR=dist_make_check','dist_make_check/VOLTAGE_SOGI_SYNC_FIXED60_NEWPI.X.production.hex']
with (R/'make_check.log').open('w') as f:p=subprocess.run(cmd,cwd=C,env=env,stdout=f,stderr=subprocess.STDOUT)
result={'returncode':p.returncode,'command':cmd}
if p.returncode==0:
 h=C/'dist_make_check/VOLTAGE_SOGI_SYNC_FIXED60_NEWPI.X.production.hex'
 result['hex_sha256']=hashlib.sha256(h.read_bytes()).hexdigest()
 result['matches_canonical_hex']=result['hex_sha256']==json.loads((R/'build_result.json').read_text())['sha256']
(R/'make_check.json').write_text(json.dumps(result,indent=2))
print(json.dumps(result,indent=2))
assert p.returncode==0
assert result['matches_canonical_hex']

