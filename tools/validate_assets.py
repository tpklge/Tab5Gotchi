#!/usr/bin/env python3
from pathlib import Path
import json,hashlib,wave,struct
import numpy as np
ROOT=Path(__file__).resolve().parents[1];base=ROOT/'sdcard/tamagotchi'
m=json.loads((base/'manifest.json').read_text());assert m['schema_version']==m['engine_api_version']==1
files=set();total=0;audio=0
for entry in m['files']:
 p=Path(entry['path']);assert not p.is_absolute() and '..' not in p.parts and str(p) not in files;files.add(str(p));data=(base/p).read_bytes();assert len(data)==entry['bytes'] and hashlib.sha256(data).hexdigest()==entry['sha256'],p;total+=len(data)
 if p.suffix=='.wav':
  with wave.open(str(base/p)) as w:
   assert (w.getnchannels(),w.getsampwidth(),w.getframerate())==(1,2,22050);a=np.frombuffer(w.readframes(w.getnframes()),dtype='<i2');assert np.any(a) and np.max(np.abs(a.astype('i4')))<30000
   if 'music' in p.parts:assert 20<=len(a)/22050<=45 and abs(int(a[0])-int(a[-1]))<32
  audio+=1
coverage=[]
for family in m['families']:
 f=json.loads((base/f'pets/{family}/family.json').read_text());assert len(f['forms'])==9 and len(f['palettes'])==4 and len(f['details'])==2
 first_poses=set()
 for i,form in enumerate(f['forms']):
  assert (base/form['file']).stat().st_size==96*96*3*32;data=(base/form['file']).read_bytes();frames=[data[j*27648:(j+1)*27648] for j in range(32)]
  assert all(any(x[18432:]) for x in frames)
  assert frames[0] not in first_poses,(family,i,'duplicate form');first_poses.add(frames[0])
  for name,seq in form['animations'].items():
   assert len(seq)>=2 and all(0<=k<32 for k in seq);assert len({frames[k] for k in seq})>=2,(family,i,name)
  coverage.append(f'| {family} | {form["name"]} | {len(form["animations"])} | 32 | 4 × 2 por composição |')
actual={str(p.relative_to(base)) for p in base.rglob('*') if p.is_file() and 'saves' not in p.parts and 'album' not in p.parts and p.name!='manifest.json'};assert actual==files,actual^files
(ROOT/'Docs/asset_coverage.md').write_text('# Cobertura de conteúdo\n\nValidada por tools/validate_assets.py. RGB565+A8 planar, 96 × 96, 160 ms por pose; passos e efeitos compõem ações.\n\n| Família | Forma | Animações | Poses | Variações |\n|---|---|---:|---:|---|\n'+'\n'.join(coverage)+f'\n\n{audio} WAVs PCM mono 22050 Hz. Total do manifesto: {total:,} bytes.\n')
print('PASS:',len(files),'arquivos,',len(coverage),'formas/ovos,',audio,'WAVs,',total,'bytes')
