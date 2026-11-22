#!/usr/bin/env python3
"""Bounded hardware regression of the current profile; restores display settings."""
import argparse,json,time,serial
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--port',default='/dev/cu.usbmodem12201');a=p.parse_args()
root=Path('private/hardware');root.mkdir(exist_ok=True);records=[]
s=serial.Serial(port=None,baudrate=115200,timeout=.2,write_timeout=5);s.rts=True;s.dtr=True;s.port=a.port;s.open();s.rts=False;s.dtr=False;time.sleep(.3);s.reset_input_buffer()
def cmd(line):
 s.write(('gotchi '+line+'\n').encode());deadline=time.monotonic()+30
 expected={'status':'profile','click':'clicked','tap':'tap','save':'saved'}.get(line.split()[0])
 while time.monotonic()<deadline:
  raw=s.readline().decode(errors='replace').strip()
  if raw.startswith('GOTCHI '):
   reply=json.loads(raw[7:])
   if expected in reply:records.append({'command':line,'reply':reply});return reply
 raise TimeoutError(line)
def state():
 r=cmd('status');assert r['ready'] and not r['fault'],r;return r
def click(code):cmd('click '+str(code));time.sleep(.22)
def shot(name):
 s.write(b'gotchi capture\n');deadline=time.monotonic()+30
 while time.monotonic()<deadline:
  raw=s.readline()
  if raw.startswith(b'GOTCHI_BINARY '):break
 else:raise TimeoutError(name)
 count=int(raw.split()[1]);data=bytearray();deadline=time.monotonic()+45
 while len(data)<count and time.monotonic()<deadline:data.extend(s.read(min(65536,count-len(data))))
 assert len(data)==count
 (root/(name+'.bmp')).write_bytes(data)
 from PIL import Image
 Image.open(root/(name+'.bmp')).save(root/(name+'.png'))
def timeout(value):
 click(81)
 while True:
  current=state()['screenTimeoutSeconds']
  if current==value:return
  click(839 if current<value else 838)
original=None
try:
 original=state();identity=original['id'];assert original['stateFormat']==2
 if original['screenOff']:cmd('tap 120 670');time.sleep(.3)
 click(17);shot('complete-album');click(46);click(850);assert state()['page']=='life';shot('complete-life')
 for _ in range(3):
  for code in [1,17,28,46,850,18,80,81]:click(code)
 assert state()['id']==identity
 timeout(30);before=state();print('TEST tela apagada, simulação e checkpoint (65 s)',flush=True)
 for i in range(13):time.sleep(5);print('aguardando',5*(i+1),flush=True)
 hidden=state();assert hidden['screenOff'] and hidden['appliedBrightness']==0
 assert hidden['age']>=before['age']+60000 and hidden['saveSequence']>before['saveSequence']
 cmd('tap 120 670');time.sleep(.4);awake=state();assert not awake['screenOff'] and awake['page']=='display' and awake['appliedBrightness']==original['brightness']
 cmd('tap 120 670');time.sleep(.4);assert state()['page']=='home'
 timeout(0);print('TEST Nunca (33 s)',flush=True)
 for _ in range(3):time.sleep(11);print('aguardando Nunca',flush=True)
 assert not state()['screenOff']
 timeout(original['screenTimeoutSeconds']);click(1)
 # A timed game measures rendering with no captures and only one status at end.
 if not original['sleeping'] and original['stage']>0:
  click(306);print('TEST Pega-frutinha e desempenho (32 s)',flush=True)
  for _ in range(4):time.sleep(8);print('jogo em execução',flush=True)
  fruit=state();assert fruit['gameFinished'] and not fruit['gameActive']
  print('FPS durante jogo:',fruit['fps'],flush=True);click(43)
  click(307);shot('complete-hide')
  for i in range(3):click(700+i);time.sleep(1.6)
  assert state()['gameFinished'];click(43)
 click(1);final=state();assert final['id']==identity and final['audioErrors']==0 and final['audioResourceErrors']==0
 shot('complete-home');cmd('save');print('PASS smoke',json.dumps(final),flush=True)
finally:
 if original:
  try:timeout(original['screenTimeoutSeconds']);click(1)
  except Exception as e:print('Restaurar timeout:',type(e).__name__,flush=True)
 s.close();(root/'complete-smoke.json').write_text(json.dumps(records,ensure_ascii=False,indent=2)+'\n')
