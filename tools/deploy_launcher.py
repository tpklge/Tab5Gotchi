#!/usr/bin/env python3
"""Install via the launcher's verified API; preserve other apps and SD progress."""
import argparse, hashlib, json, sys, time, urllib.parse, urllib.request
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--port',default='/dev/cu.usbmodem12201');p.add_argument('--slot',type=int,default=2);p.add_argument('--content',action='store_true');p.add_argument('--content-only',action='store_true');p.add_argument('--boot',action='store_true');a=p.parse_args()
from launcher_client import Tab5
s=Tab5(a.port,timeout=30)
try:
 state=s.command('status')
 if state.get('wifiState')!=7:s.command('wifi-retry')
 for attempt in range(10):
  try: w=s.command('web-start')
  except RuntimeError:
   print('Aguardando inicialização do launcher',flush=True);time.sleep(3);continue
  if w.get('ip') and w['ip']!='0.0.0.0':break
  print('Aguardando Wi-Fi do launcher',flush=True);time.sleep(3)
 else:raise TimeoutError('Launcher sem endereço de rede')
finally:s.close()
base='http://'+w['ip'];token=w['code']
def req(route,data=None,raw=False):
 h={'X-Launcher-Token':token}
 if data is not None and not isinstance(data,bytes):data=json.dumps(data).encode();h['Content-Type']='application/json'
 with urllib.request.urlopen(urllib.request.Request(base+route,data=data,headers=h),timeout=60) as r:b=r.read()
 return b if raw else json.loads(b)
def operation(data):
 plan=req('/api/operation',data);assert plan.get('ok'),plan
 print('PLAN',plan['message'],flush=True)
 if data['action']=='install':assert f"slot {a.slot} (app{a.slot})" in plan['message'],plan
 req('/api/operation',{'action':'execute','ticket':plan['ticket']})
 for _ in range(120):
  time.sleep(1)
  try: result=req('/api/operation')
  except (OSError, TimeoutError):
   if not plan.get('willRestart'): raise
   print('Aguardando reinício e retomada pelo launcher', flush=True)
   time.sleep(8)
   device=Tab5(a.port,timeout=15)
   try: fresh=device.command('web-start')
   finally: device.close()
   global base,token
   if not fresh.get('ip') or fresh['ip']=='0.0.0.0': continue
   base='http://'+fresh['ip'];token=fresh['code']
   continue
  if not result.get('running'):
   assert result.get('error',0)==0,result
   if plan.get('willRestart') and result.get('percent',0)==0:
    next_plan=req('/api/operation',data)
    assert next_plan.get('ok'),next_plan
    assert not next_plan.get('willRestart'), 'Layout ainda pendente; não repetir migração'
    req('/api/operation',{'action':'execute','ticket':next_plan['ticket']})
    plan=next_plan
    continue
   print(result,flush=True);return result
 raise TimeoutError('Operação pendente; confira launcher')
def upload(remote,data):
 folder,name=remote.rsplit('/',1)
 r=req('/api/upload?'+urllib.parse.urlencode({'directory':folder,'relative':name}),data);assert r.get('verified'),r
 actual=req('/api/file?'+urllib.parse.urlencode({'path':remote}),raw=True)
 assert hashlib.sha256(actual).digest()==hashlib.sha256(data).digest(),remote
 print('VERIFIED',remote,len(data),flush=True)
backup=Path('private/hardware')/('stage-'+str(time.time_ns()));backup.mkdir(parents=True)
apps=req('/api/apps');(backup/'apps-before.json').write_text(json.dumps(apps,indent=2))
for e in req('/api/files?'+urllib.parse.urlencode({'path':'/sdcard/tamagotchi/saves','all':1}))['entries']:
 if not e['directory']:
  data=req('/api/file?'+urllib.parse.urlencode({'path':'/sdcard/tamagotchi/saves/'+e['name']}),raw=True)
  (backup/e['name']).write_bytes(data)
print('BACKUP',backup,flush=True)
if a.content or a.content_only:
 root=Path('sdcard/tamagotchi');new=json.loads((root/'manifest.json').read_text());old=req('/api/file?'+urllib.parse.urlencode({'path':'/sdcard/tamagotchi/manifest.json'}));hashes={e['path']:e['sha256'] for e in old['files']}
 for e in new['files']:
  if hashes.get(e['path'])!=e['sha256']:
   assert not {'saves','album'}&set(Path(e['path']).parts)
   upload('/sdcard/tamagotchi/'+e['path'],(root/e['path']).read_bytes())
 upload('/sdcard/tamagotchi/manifest.json',(root/'manifest.json').read_bytes())
if a.content_only:raise SystemExit(0)
image=Path('build/tab5gotchi.bin').read_bytes();assert image[0]==0xe9 and image[80:112].split(b'\0')[0]==b'tab5gotchi';remote='/sdcard/APP/Tab5Gotchi-'+hashlib.sha256(image).hexdigest()[:12]+'.bin';upload(remote,image)
target=next((x for x in apps if x['slot']==a.slot),None)
if target and target.get('descriptorValid'):
 assert target['name']=='tab5gotchi',target
 operation({'action':'uninstall','slot':a.slot,'backupBinary':True,'backupData':False})
result=operation({'action':'install','path':remote});(backup/'install-result.json').write_text(json.dumps(result,indent=2))
after=req('/api/apps');assert next(x for x in after if x['slot']==a.slot)['name']=='tab5gotchi'
for app in apps:
 if app['slot']!=a.slot:assert next((x for x in after if x['slot']==app['slot']),None)==app
if a.boot:
 s=Tab5(a.port)
 try:print(s.command('boot',a.slot))
 finally:s.close()
