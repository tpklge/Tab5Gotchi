"""Deterministic scene props and declarative tuning for the complete runtime."""
from pathlib import Path
import json,hashlib
from PIL import Image,ImageDraw
import numpy as np
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'sdcard/tamagotchi'
def write(path,obj):path.write_text(json.dumps(obj,ensure_ascii=False,indent=2)+'\n')
def raw(image):
 a=np.asarray(image,dtype=np.uint16);rgb=((a[:,:,0]>>3)<<11)|((a[:,:,1]>>2)<<5)|(a[:,:,2]>>3)
 return rgb.astype('<u2').tobytes()+a[:,:,3].astype('u1').tobytes()
contact=Image.new('RGBA',(128*3,96*2),'#f0eadb')
for i in range(6):
 im=Image.new('RGBA',(128,96));d=ImageDraw.Draw(im);ink='#34384e'
 if i==0:
  d.polygon([(34,45),(96,45),(85,91),(45,91)],fill='#bc8868',outline=ink,width=3);d.line((64,48,64,16),fill='#598765',width=5)
  d.ellipse((20,10,66,40),fill='#87b079',outline=ink,width=2);d.ellipse((63,3,108,33),fill='#a8c78b',outline=ink,width=2)
 elif i==1:
  d.rectangle((57,30,72,94),fill='#a38365',outline=ink,width=3)
  for x,y in [(19,25),(61,22),(37,0)]:d.ellipse((x,y,x+53,y+53),fill='#7da378',outline=ink,width=2)
 elif i==2:
  for x in [25,99]:d.rectangle((x,51,x+6,94),fill='#8c705b',outline=ink)
  d.rounded_rectangle((10,35,119,65),5,fill='#b49570',outline=ink,width=3)
  d.rectangle((16,17,112,32),fill='#c9aa80',outline=ink,width=2)
 elif i==3:
  d.rectangle((24,24,108,91),fill='#bd9974',outline=ink,width=3)
  d.polygon([(24,24),(10,8),(55,4),(66,24),(105,3),(119,15),(108,24)],fill='#dbc29e',outline=ink,width=3)
  d.line((66,25,66,91),fill='#987450',width=3)
 elif i==4:
  for x,y in [(9,40),(44,18),(77,39)]:d.ellipse((x,y,x+45,y+50),fill='#88ae7f',outline=ink,width=2)
  d.ellipse((31,55,99,95),fill='#709764',outline=ink,width=2)
 else:
  d.ellipse((8,57,120,94),fill='#b4c9ca',outline=ink,width=3);d.ellipse((18,63,110,86),fill='#7fb7ce',outline=ink,width=2)
  d.rectangle((57,23,71,69),fill='#a2babf',outline=ink,width=2);d.ellipse((36,17,92,37),fill='#b4c9ca',outline=ink,width=2)
  d.arc((40,0,87,33),180,355,fill='#78b4cb',width=4);d.line((64,4,64,21),fill='#91cddd',width=3)
 (OUT/f'ui/hide{i}.rgb').write_bytes(raw(im));contact.alpha_composite(im,((i%3)*128,(i//3)*96))
contact.convert('RGB').save(ROOT/'previews/esconde-objetos.png')
r=json.loads((OUT/'data/rules.json').read_text());r.update(social_minimum_scale=0.25,action_cooldown_seconds=60,education_cooldown_seconds=300,reward_cooldown_seconds=300,action_duration_ms=1300,explore_duration_ms=4000,nap_duration_seconds=600,evolution_activity_cap=40,evolution_weights=[2,5,3],game_reward=[4,2,2,3],action_gains=[[24,5,1],[14,5,1],[6,5,1],[2,0,0],[3,2,0],[3,0,0],[0,0,0],[10,0,0],[0,0,0],[0,0,0],[0,0,0],[100,5,0],[4,0,0],[0,0,0],[18,5,0],[3,1,0],[2,2,0],[4,6,0],[3,2,0],[2,0,0]])
write(OUT/'data/rules.json',r)
for file in (OUT/'pets').glob('*/family.json'):
 family=json.loads(file.read_text())
 for form in family['forms']:form['frame_ms']=160
 write(file,family)
m=json.loads((OUT/'manifest.json').read_text());m['content_version']='0.2.0';m['files']=[{'path':str(p.relative_to(OUT)),'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in sorted(OUT.rglob('*')) if p.is_file() and p.name!='manifest.json' and 'saves' not in p.parts and 'album' not in p.parts]
write(OUT/'manifest.json',m)
