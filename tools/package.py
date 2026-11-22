#!/usr/bin/env python3
"""Package checked application binaries, immutable SD content, sources and licenses.
Never includes private backups, build directories, live saves or credentials.
"""
from pathlib import Path
import json,hashlib,zipfile,shutil,struct
ROOT=Path(__file__).resolve().parents[1];DIST=ROOT/'dist';DIST.mkdir(exist_ok=True)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def zip_files(target,files):
 with zipfile.ZipFile(target,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=9) as z:
  for p,arc in sorted(files,key=lambda x:x[1]):z.write(p,arc)
def safe_files(base):
 return [p for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts and not p.name.startswith('._')]
app=ROOT/'build/tab5gotchi.bin';data=app.read_bytes();assert data[0]==0xe9 and struct.unpack_from('<I',data,32)[0]==0xabcd5432
shutil.copyfile(app,DIST/'tab5gotchi-app.bin')
content=ROOT/'sdcard/tamagotchi';manifest=json.loads((content/'manifest.json').read_text());sd_files=[]
for e in manifest['files']:
 p=content/e['path'];assert p.stat().st_size==e['bytes'] and sha(p)==e['sha256'];assert 'saves' not in p.relative_to(content).parts and 'album' not in p.relative_to(content).parts;sd_files.append((p,'tamagotchi/'+e['path']))
for name in ['manifest.json','saves/LEIA-ME.txt','album/LEIA-ME.txt']:sd_files.append((content/name,'tamagotchi/'+name))
zip_files(DIST/'sdcard.zip',sd_files)
licenses=DIST/'licenses';licenses.mkdir(exist_ok=True)
for name in ['LICENSE','LICENSE-MIT','THIRD_PARTY_NOTICES.md']:shutil.copyfile(ROOT/name,licenses/name)
shutil.copytree(ROOT/'assets_src/fonts',licenses/'DejaVu-source',dirs_exist_ok=True)
# Preserve upstream license/notice texts without distributing SDK build products.
for base in [ROOT/'managed_components',ROOT/'private/extras-source/managed_components']:
 if not base.exists():continue
 for component in base.iterdir():
  if not component.is_dir():continue
  for p in component.iterdir():
   if p.is_file() and any(p.name.lower().startswith(x) for x in ['license','licence','copying','notice']):
    target=licenses/component.name;target.mkdir(exist_ok=True);shutil.copyfile(p,target/p.name)
font_licenses=ROOT/'managed_components/lvgl__lvgl/scripts/built_in_font/font_license'
if font_licenses.exists():shutil.copytree(font_licenses,licenses/'LVGL-fonts',dirs_exist_ok=True)
idf_license=Path.home()/'esp/esp-idf/LICENSE'
if idf_license.exists():shutil.copyfile(idf_license,licenses/'ESP-IDF-LICENSE')
idf_components=idf_license.parent/'components'
if idf_components.exists():
 for p in idf_components.rglob('*'):
  if p.is_file() and any(p.name.lower().startswith(x) for x in ['license','licence','copying','notice']):
   target=licenses/'ESP-IDF-components'/p.relative_to(idf_components);target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(p,target)
source=[]
for directory in ['main','tools','tests','Docs','assets_src','previews']:
 source.extend((p,str(p.relative_to(ROOT))) for p in safe_files(ROOT/directory))
for name in ['CMakeLists.txt','partitions.csv','sdkconfig.defaults','sdkconfig.extras','sdkconfig','dependencies.lock','README.md','LICENSE','LICENSE-MIT','THIRD_PARTY_NOTICES.md','requirements-tools.txt','.gitignore']:
 p=ROOT/name
 if p.exists():source.append((p,name))
extra_lock=ROOT/'private/extras-source/dependencies.lock'
if extra_lock.exists():source.append((extra_lock,'dependencies-extras.lock'))
zip_files(DIST/'source.zip',source)
summary={'version':data[48:80].split(b'\0')[0].decode('ascii'),'target':'esp32p4','type':'application-only','families':6,'forms':48,'eggs':6,'games':8,'content_bytes':sum(e['bytes'] for e in manifest['files']),'immutable_files':len(manifest['files']),'firmware_bytes':app.stat().st_size,'game_assets_embedded':False,'diagnostic_font_in_flash':True,'sd_preserve':['tamagotchi/saves','tamagotchi/album'],'hardware_evidence':'Docs/hardware.md','files':[]}
for name in ['tab5gotchi-app.bin','tab5gotchi-extras-app.bin','sdcard.zip','source.zip']:
 p=DIST/name
 if p.exists():summary['files'].append({'path':name,'bytes':p.stat().st_size,'sha256':sha(p)})
(DIST/'manifest.json').write_text(json.dumps(summary,ensure_ascii=False,indent=2)+'\n')
artifacts=[DIST/e['path'] for e in summary['files']]+[DIST/'manifest.json']
(DIST/'SHA256SUMS').write_text(''.join(f'{sha(p)}  {p.name}\n' for p in artifacts))
for name in ['README.md','THIRD_PARTY_NOTICES.md']:shutil.copyfile(ROOT/name,DIST/name)
release=[(p,p.name) for p in artifacts+[DIST/'SHA256SUMS',DIST/'README.md',DIST/'THIRD_PARTY_NOTICES.md']]+[(p,str(p.relative_to(DIST))) for p in safe_files(licenses)]
zip_files(DIST/'tab5gotchi-release.zip',release)
# Read every archive member and CRC; assert mutable progress was excluded.
for name in ['sdcard.zip','source.zip','tab5gotchi-release.zip']:
 with zipfile.ZipFile(DIST/name) as z:
  assert z.testzip() is None
  assert not any('/private/' in n or n.startswith('private/') or n.endswith('.sav') for n in z.namelist())
(DIST/'tab5gotchi-release.zip.sha256').write_text(f"{sha(DIST/'tab5gotchi-release.zip')}  tab5gotchi-release.zip\n")
print(json.dumps(summary,ensure_ascii=False,indent=2));print('Release ZIP:',(DIST/'tab5gotchi-release.zip').stat().st_size,'bytes')
