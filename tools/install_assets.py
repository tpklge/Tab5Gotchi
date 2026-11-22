#!/usr/bin/env python3
"""Copy immutable content, preserve progress, verify bytes. Defaults to dry-run."""
from pathlib import Path
import argparse,json,hashlib,shutil
p=argparse.ArgumentParser();p.add_argument('mount',type=Path);p.add_argument('--apply',action='store_true');a=p.parse_args();root=Path(__file__).resolve().parents[1]/'sdcard/tamagotchi';mount=a.mount.resolve()
if not mount.is_dir():p.error('Destino inexistente')
if not (mount/'launcher').is_dir():p.error('Destino precisa ser o cartão Tab5 identificado pela pasta launcher')
m=json.loads((root/'manifest.json').read_text());files=[x['path'] for x in m['files']]+['manifest.json']
for name in files:
 parts=Path(name).parts
 if Path(name).is_absolute() or '..' in parts or 'saves' in parts or 'album' in parts:raise ValueError(name)
 source=root/name;target=mount/'tamagotchi'/name
 if a.apply:
  if target.is_file() and hashlib.sha256(source.read_bytes()).digest()==hashlib.sha256(target.read_bytes()).digest():continue
  target.parent.mkdir(parents=True,exist_ok=True);tmp=target.with_name(target.name+'.new');shutil.copyfile(source,tmp)
  if hashlib.sha256(source.read_bytes()).digest()!=hashlib.sha256(tmp.read_bytes()).digest():raise IOError(name)
  tmp.replace(target)
if a.apply:
 for name in ['saves','album']:(mount/'tamagotchi'/name).mkdir(exist_ok=True)
print(('INSTALADO E VERIFICADO' if a.apply else 'DRY-RUN'),len(files),'arquivos em',mount/'tamagotchi','; saves e álbum preservados')
