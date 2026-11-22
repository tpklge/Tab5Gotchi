#!/usr/bin/env python3
"""Update only an existing Tab5Gotchi slot after reading the installed map.
Run with ESP-IDF Python. Never changes bootloader, partition table or other apps.
"""
from pathlib import Path
import argparse,subprocess,sys,os,struct,hashlib,json,time
p=argparse.ArgumentParser();p.add_argument('--port',required=True);p.add_argument('--slot',type=int,required=True);p.add_argument('--binary',type=Path,default=Path('build/tab5gotchi.bin'));p.add_argument('--apply',action='store_true');a=p.parse_args()
if not 0<=a.slot<16:p.error('slot inválido')
root=Path(__file__).resolve().parents[1];private=root/'private/hardware';private.mkdir(parents=True,exist_ok=True);stamp=str(time.time_ns());table=private/f'table-{stamp}.bin'
def esp(*args):subprocess.run([sys.executable,'-m','esptool','--chip','esp32p4','--port',a.port,'--baud','460800',*map(str,args)],check=True)
esp('read_flash','0x8000','0x1000',table)
sys.path.insert(0,str(Path(os.environ['IDF_PATH'])/'components/partition_table'))
from gen_esp32part import PartitionTable
parts=PartitionTable.from_binary(table.read_bytes()[:0xc00]);parts.verify();part=next((x for x in parts if x.type==0 and x.subtype==0x10+a.slot),None)
if not part:raise SystemExit('Slot não existe. Instale primeiro pelo launcher.')
backup=private/f'app{a.slot}-before-{stamp}.bin';esp('read_flash',hex(part.offset),hex(part.size),backup)
def descriptor(data):
 if len(data)<256 or data[0]!=0xe9 or struct.unpack_from('<I',data,32)[0]!=0xabcd5432:raise ValueError('Imagem ESP sem descritor')
 return data[80:112].split(b'\0')[0].decode('ascii')
old=descriptor(backup.read_bytes());data=a.binary.read_bytes();new=descriptor(data)
if old!='tab5gotchi' or new!='tab5gotchi':raise SystemExit('Somente um slot já identificado como tab5gotchi pode ser atualizado por esta ferramenta.')
if len(data)>part.size:raise SystemExit('Imagem não cabe no slot; use o gerenciador de aplicativos do launcher.')
plan={'slot':a.slot,'label':part.name,'address':hex(part.offset),'reservation':part.size,'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest(),'existingProject':old,'preserved':['bootloader','partition table','launcher','all other slots','SD saves']}
(private/'last-update.json').write_text(json.dumps(plan,indent=2)+'\n');print(json.dumps(plan,indent=2))
if a.apply:esp('write_flash',hex(part.offset),a.binary)
else:print('DRY-RUN: nenhuma gravação. Use --apply para gravar apenas este aplicativo.')
