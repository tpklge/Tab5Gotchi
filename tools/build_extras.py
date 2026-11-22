#!/usr/bin/env python3
"""Build optional LAN/breeding in an isolated source tree, without touching base dependencies."""
from pathlib import Path
import shutil,subprocess,sys
root=Path(__file__).resolve().parents[1];dest=root/'private/extras-source';dest.mkdir(parents=True,exist_ok=True)
if not (dest/'dependencies.lock').exists() and (root/'dependencies-extras.lock').exists():
 shutil.copyfile(root/'dependencies-extras.lock',dest/'dependencies.lock')
for directory in ['main','tools']:shutil.copytree(root/directory,dest/directory,dirs_exist_ok=True)
for name in ['CMakeLists.txt','partitions.csv']:shutil.copyfile(root/name,dest/name)
project=(dest/'CMakeLists.txt').read_text().replace('set(PROJECT_VER "0.2.1")','set(PROJECT_VER "0.2.1-extras")');(dest/'CMakeLists.txt').write_text(project)
(dest/'sdkconfig.defaults').write_text((root/'sdkconfig.defaults').read_text()+'\n'+(root/'sdkconfig.extras').read_text())
manifest=(root/'main/idf_component.yml').read_text()+"  espressif/esp_hosted: '==2.13.1'\n  espressif/esp_wifi_remote: '==1.6.5'\n"
(dest/'main/idf_component.yml').write_text(manifest)
if not (dest/'build/config/sdkconfig.h').is_file():
 subprocess.run(['idf.py','-C',str(dest),'set-target','esp32p4'],check=True)
else:
 import re
 config=(dest/'sdkconfig').read_text()
 for line in (dest/'sdkconfig.defaults').read_text().splitlines():
  if line.startswith('CONFIG_') and '=' in line:
   key=line.split('=',1)[0]
   config=re.sub(r'^(?:# )?'+re.escape(key)+r'(?:=.*| is not set)$',line,config,flags=re.M) if re.search(r'^(?:# )?'+re.escape(key)+r'(?:=| is not set)',config,re.M) else config+'\n'+line+'\n'
 (dest/'sdkconfig').write_text(config)
subprocess.run(['idf.py','-C',str(dest),'build'],check=True)
(root/'dist').mkdir(exist_ok=True)
shutil.copyfile(dest/'build/tab5gotchi.bin',root/'dist/tab5gotchi-extras-app.bin')
print('Optional app-only build:',root/'dist/tab5gotchi-extras-app.bin')
