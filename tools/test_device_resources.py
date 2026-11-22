#!/usr/bin/env python3
"""Reversible resource-fault tests. Backs up and restores resources; never edits saves."""
import hashlib,json,time,sys,urllib.request,urllib.parse
from pathlib import Path
from launcher_client import Tab5
root=Path('private/hardware/resource-test');root.mkdir(parents=True,exist_ok=True)
records=[]
def launcher():
    retried_wifi=False
    for attempt in range(12):
        s=Tab5('/dev/cu.usbmodem12201',timeout=3)
        try:
            s.serial.write(b'gotchi launcher\n');time.sleep(1.5)
            try:
                status=s.command('status')
                if status.get('wifiState')!=7:
                    if not retried_wifi:s.command('wifi-retry');retried_wifi=True
                    time.sleep(3);continue
                w=s.command('web-start')
                if w.get('ip') and w['ip']!='0.0.0.0':return w
            except (RuntimeError,TimeoutError):pass
        finally:s.close()
        print('Aguardando launcher',attempt+1,flush=True);time.sleep(2)
    raise TimeoutError('Launcher sem WebGUI')
def api(w,route,data=None,raw=False):
    payload=data
    if payload is not None and not isinstance(payload,bytes):payload=json.dumps(payload).encode()
    for attempt in range(4):
        h={'X-Launcher-Token':w['code']}
        if data is not None and not isinstance(data,bytes):h['Content-Type']='application/json'
        try:
            with urllib.request.urlopen(urllib.request.Request('http://'+w['ip']+route,data=payload,headers=h),timeout=45) as r:b=r.read()
            return b if raw else json.loads(b)
        except (OSError,TimeoutError):
            if attempt==3:raise
            print('Reconectando WebGUI',flush=True);time.sleep(2);w.update(launcher())
def read(w,path):return api(w,'/api/file?'+urllib.parse.urlencode({'path':path}),raw=True)
def upload(w,path,data):
    data=bytes(data)
    folder,name=path.rsplit('/',1)
    r=api(w,'/api/upload?'+urllib.parse.urlencode({'directory':folder,'relative':name}),data);assert r['verified']
    assert read(w,path)==data
    print('VERIFIED',path,flush=True)
def boot():
    s=Tab5('/dev/cu.usbmodem12201')
    try:assert not s.command('boot',2)['error']
    finally:s.close()
    time.sleep(3)
def game(command='status',key='profile'):
    s=Tab5('/dev/cu.usbmodem12201',timeout=20)
    try:
        s.serial.write(('gotchi '+command+'\n').encode());deadline=time.monotonic()+30
        while time.monotonic()<deadline:
            line=s.serial.readline().decode(errors='replace').strip()
            if line.startswith('GOTCHI '):
                reply=json.loads(line[7:])
                if key in reply:return reply
        raise TimeoutError(command)
    finally:s.close()
def saves(w):
    names=api(w,'/api/files?'+urllib.parse.urlencode({'path':'/sdcard/tamagotchi/saves','all':1}))['entries']
    return {e['name']:hashlib.sha256(read(w,'/sdcard/tamagotchi/saves/'+e['name'])).hexdigest() for e in names if not e['directory']}
w=launcher();paths=['/sdcard/tamagotchi/audio/sfx/botao.wav','/sdcard/tamagotchi/ui/hide0.rgb','/sdcard/tamagotchi/manifest.json'];originals={p:read(w,p) for p in paths}
for path,data in originals.items():(root/Path(path).name).write_bytes(data)
modified=False
try:
    if '--only-compatibility' not in sys.argv:
        for path in paths[:2]:
            damaged=bytearray(originals[path]);damaged[60]^=1;upload(w,path,damaged);modified=True
        boot();first=game();assert first['ready'] and not first['fault']
        game('click 18','clicked');time.sleep(.5);game('click 307','clicked');time.sleep(.6)
        result=game();assert result['ready'] and not result['fault'] and result['audioResourceErrors']>0
        records.append({'test':'corrupt audio/image','state':result});print('PASS áudio/imagem corrompidos sem interromper simulação',flush=True)
        w=launcher()
        for path in paths[:2]:upload(w,path,originals[path])
    before=saves(w)
    manifest=json.loads(originals[paths[2]]);manifest['engine_api_version']=999
    upload(w,paths[2],json.dumps(manifest).encode());boot();blocked=game();assert not blocked['ready']
    records.append({'test':'incompatible content','state':blocked});w=launcher();assert saves(w)==before
    print('PASS pacote incompatível bloqueado sem alterar saves',flush=True)
finally:
    w=launcher()
    for path,data in originals.items():upload(w,path,data)
    boot();restored=game();game('click 43','clicked');game('click 1','clicked');restored=game();records.append({'test':'restored','state':restored})
    assert restored['ready'] and not restored['fault'] and restored['audioResourceErrors']==0
    if records and records[0].get('state',{}).get('ready'):assert restored['id']==records[0]['state']['id']
    (root/'results.json').write_text(json.dumps(records,ensure_ascii=False,indent=2)+'\n')
    print('PASS recursos restaurados e pet recuperado',flush=True)
