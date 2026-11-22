#!/usr/bin/env python3
import argparse,json,serial,time
from pathlib import Path
from PIL import Image
parser=argparse.ArgumentParser(description='Regressão no Tab5: altera os três perfis de teste; requer pet nascido e A/B/C desligado.')
parser.add_argument('--port',default='/dev/cu.usbmodem12201');args=parser.parse_args()
Path('private/hardware').mkdir(parents=True,exist_ok=True)
s=serial.Serial(port=None,baudrate=115200,timeout=.2,write_timeout=5);s.rts=True;s.dtr=True;s.port=args.port;s.open();s.rts=False;s.dtr=False;time.sleep(.2);s.reset_input_buffer();records=[]
def cmd(t,delay=.06):
 s.write(('gotchi '+t+'\n').encode());deadline=time.monotonic()+20
 wanted={'status':'profile','click':'clicked','tap':'tap','save':'saved'}[t.split()[0]]
 while time.monotonic()<deadline:
  line=s.readline().decode(errors='replace').strip()
  if line.startswith('METRICS '):records.append({'metrics':line})
  if line.startswith('GOTCHI '):
   data=json.loads(line[7:])
   if wanted in data:
    records.append({'command':t,'reply':data});time.sleep(delay);return data
 raise TimeoutError(t)
def status():
 x=cmd('status');assert x['ready'] and not x['fault'],x;return x
def click(n,delay=.25):return cmd('click '+str(n),delay)
def shot(name):
 s.write(b'gotchi capture\n');deadline=time.monotonic()+30
 while time.monotonic()<deadline:
  line=s.readline()
  if line.startswith(b'GOTCHI_BINARY '):break
  if b'captureError' in line:raise RuntimeError(line)
 else:raise TimeoutError('capture '+name)
 n=int(line.split()[1]);data=bytearray();deadline=time.monotonic()+50
 while len(data)<n and time.monotonic()<deadline:data.extend(s.read(min(n-len(data),65536)))
 assert len(data)==n,(name,len(data),n)
 file=Path('private/hardware/verified-'+name+'.bmp');file.write_bytes(data);Image.open(file).save(file.with_suffix('.png'))
def finish_check(before,game):
 x=status();assert x['game']==game and x['gameFinished'] and not x['gameActive'] and x['gameRewarded'] and x['gameCount']==before+1,x
 click(700);assert status()['gameCount']==before+1
 print('PASS jogo',game,'placar',x['gameWins'],x['gameLosses'],'pontos',x['gameScore'],'recompensa única',flush=True)
 if game in [1,5]:shot('jogo-'+str(game))
 click(43)
try:
 click(510,.6);click(1);start=status();identity=start['id'];assert start['stage']>=1
 print('Perfil inicial recuperado',identity,'seq',start['saveSequence'],flush=True)
 cmd('tap 1040 145',.5);assert status()['page']=='food'
 click(200,1.4);assert status()['stats'][0]<=100;click(104,1.4)
 click(12);click(113,1.4);assert status()['sleeping']==1;click(119,1.4);click(114,1.4);assert not status()['sleeping']
 click(120,1.4);assert status()['sleeping'];click(114,1.4);assert not status()['sleeping']
 for p in [1,2,0]:click(510+p,.6);assert status()['profile']==p
 assert status()['id']==identity
 print('PASS cuidados iniciais, eventos de toque LVGL e três perfis',flush=True)
 # Complete the four games based on public choices and score.
 for index in range(4):
  before=status()['gameCount'];click(300+index,.3);attempts=0
  while status()['gameActive']:
   x=status();choice=attempts%3 if index==0 else attempts%6 if index==1 else attempts%2 if index==2 else (0 if x['gameBase']==9 else 1)
   click(700+choice,1.6);attempts+=1;assert attempts<45
  finish_check(before,index+1)
 # Six pairs, learning only the faces currently visible on screen.
 before=status()['gameCount'];click(304);click(318,.3);known={};turns=0
 shot('memoria-oito-pares')
 while status()['gameActive']:
  x=status();matched=x['memoryMatched'];available=[i for i,v in enumerate(matched) if not v]
  pairs=[(a,b) for a in available for b in available if a<b and a in known and b in known and known[a]==known[b]]
  first=pairs[0][0] if pairs else next((i for i in available if i not in known),available[0]);click(700+first,.08);x=status();known[first]=x['memoryShown'][first]
  second=pairs[0][1] if pairs else next((i for i in available if i!=first and known.get(i)==known[first]),next(i for i in available if i!=first and i not in known) if any(i!=first and i not in known for i in available) else next(i for i in available if i!=first))
  click(700+second,.08);x=status();known[second]=x['memoryShown'][second];time.sleep(1.05);turns+=1;assert turns<60
 finish_check(before,5)
 # Observe the same illuminated notes the player sees, including dark gaps.
 before=status()['gameCount'];click(305,.08);sequence=[];lit=-1;level=1;deadline=time.monotonic()+150
 while True:
  x=status()
  if not x['gameActive']:break
  assert time.monotonic()<deadline
  if x['sequenceShowing']:
   if x['sequenceLength']!=level:sequence=[];lit=-1;level=x['sequenceLength'];print('Sequência musical nível',level,flush=True)
   note=x['sequenceLit']
   if note>=0 and lit<0:sequence.append(note)
   lit=note;time.sleep(.06)
  else:
   assert len(sequence)==x['sequenceLength'],(sequence,x)
   for note in sequence:click(700+note,.1)
   sequence=[];lit=-1
 finish_check(before,6)
 before=status()['gameCount'];click(306,.1);deadline=time.monotonic()+40
 while True:
  x=status()
  if not x['gameActive']:break
  assert time.monotonic()<deadline
  px=46+x['fruitX']*830//640;cmd('tap '+str(px)+' 560',.32)
 fruit=status();assert fruit['gameScore']>0,fruit;finish_check(before,7)
 before=status()['gameCount'];click(307)
 while status()['gameActive']:x=status();click(700+x['gameRound']%6,1.6)
 finish_check(before,8)
 # Full hygiene after the earlier cooldowns, plus medicine refusal.
 click(11);click(109,.3);assert status()['bathStep']==1
 for a,step in [(110,2),(111,3),(112,0)]:click(a,.3);assert status()['bathStep']==step
 assert status()['stats'][3]>99;click(115,1.4)
 # Export/restore requires confirmation and preserves the profile identity.
 click(512,.5);p2=status();click(38);click(501);assert status()['mode']==1;click(39);assert status()['page']=='confirm' and status()['mode']==1;click(900,.7);assert status()['mode']==0 and status()['id']==p2['id'];click(502);assert status()['mode']==0;click(1)
 click(510,.6);assert status()['id']==identity;print('PASS banho, recuperação, confirmação e perfis separados',flush=True)
 click(18);shot('ajustes');click(1)
 click(16);shot('info');click(24);click(17);shot('album');click(1);shot('home')
 final=status();assert final['id']==identity and final['saveSequence']>start['saveSequence'] and final['audioErrors']==0;cmd('save');print('PASS FINAL',json.dumps(final),flush=True)
finally:
 s.close();Path('private/hardware/verification.json').write_text(json.dumps(records,ensure_ascii=False,indent=2)+'\n')
