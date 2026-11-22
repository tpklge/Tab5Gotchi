#!/usr/bin/env python3
"""Original offline pixel art, runtime RGB565+A8 banks, bitmap font and PCM audio."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
import numpy as np
import json, math, wave, hashlib, struct, shutil
ROOT=Path(__file__).resolve().parents[1]; OUT=ROOT/'sdcard/tamagotchi'; PRE=ROOT/'previews'
FAMILIES=[('capim','Capim',['Capipim','Capimirim','Capicalmo','Capicurioso','Capimanso','Capitrilha','Capifesta','Capivô']),( 'brotito','Brotito',['Sementim','Brotinho','Folhanso','Ramisco','Florim','Galhote','Frutino','Tronquim']),('brasquito','Brasquito',['Faisquito','Brasquim','Brasmanso','Brascout','Cristalume','Rocadraco','Faiscão','Brasancião']),('gotim','Gotim',['Gotícula',"Pinguim-d’água",'Ondim','Saltágua','Coralino','Marulhão','Bolhudo','Gotavô']),('nuvito','Nuvito',['Flocim','Nuvim','Brisanso','Ventito','Pluminho','Tempestino','Arcoíro','Cirrô']),('parafim','Parafim',['Porquinha','Engrenim','Calmobô','Curiosô','Serenóide','Voltix','Ritmô','Ferruvô'])]
COLORS=['#bb8758','#79b950','#e88b58','#65c4d1','#c1bddd','#e5be61']; DARK='#34384e'; LIGHT='#fff7dc'
ANIMS={'idle':[0,1,2,3],'piscar':[0,28,0],'olhar':[0,29,30],'andar':list(range(4,10)),'feliz':[10,11,10],'triste':[12,13],'fome':[12,14,13],'cansaco':[18,28,19],'dormir':[18,19],'comer':[14,15,14,31],'beber':[16,17,16,31],'urgencia':[22,23,12],'banheiro':[22,23,22],'banho':[20,21,20,31],'doente':[24,25],'vitoria':[10,11,26],'derrota':[12,13,25],'carinho':[26,27,11],'evolucao':[10,26,11,27]}
for p in ['pets','scenes','audio/sfx','audio/music','ui','data','saves','album']: (OUT/p).mkdir(parents=True,exist_ok=True)
PRE.mkdir(exist_ok=True)
fontpath=ROOT/'assets_src/fonts/DejaVuSans.ttf'
FONT=ImageFont.truetype(str(fontpath),14)
def raw(img):
 a=np.asarray(img.convert('RGBA'),dtype=np.uint16); rgb=((a[:,:,0]>>3)<<11)|((a[:,:,1]>>2)<<5)|(a[:,:,2]>>3)
 return rgb.astype('<u2').tobytes()+a[:,:,3].astype('u1').tobytes()
def pet(fam,form,frame,detail=0):
 im=Image.new('RGBA',(96,96));d=ImageDraw.Draw(im); color=COLORS[fam]; shade=tuple(int(x*.76) for x in ImageColor(color)); hi=tuple(min(255,int(x*1.14)) for x in ImageColor(color));
 y=[1,0,-1,0][frame%4];walk=4<=frame<=9;happy=frame in [10,11,26,27];sad=frame in [12,13,24,25];sleep=frame in [18,19];eat=frame in [14,15,31]
 y+=(-3 if happy and frame%2 else 0);grow=[-8,-4,0,0,3,3,3,2,0][form];cx=48;cy=53+y;w=23+grow;h=20+grow
 def ell(box,fill,outline=DARK,width=2):d.ellipse(box,fill=fill,outline=outline,width=width)
 def poly(points,fill,outline=DARK):d.polygon(points,fill=fill);d.line(points+[points[0]],fill=outline,width=2)
 if form==8:
  ell((25+y,24,71+y,83),LIGHT);d.arc((29+y,30,67+y,60),180,340,fill=color,width=5);d.rectangle((38,40+frame%4,43,46+frame%4),fill=color);d.rectangle((53,56,59,62),fill=color)
  if frame>=12:d.line([(39,28),(48,40),(43,47),(54,55),(47,66)],fill=DARK,width=2)
  if frame>=24:d.polygon([(24,50),(45,59),(72,46),(73,83),(25,83)],fill=color);d.line([(24,50),(45,59),(72,46)],fill=DARK,width=2)
  return im
 # Grounded legs with walking cycle.
 for x in [cx-w+7,cx+w-9]:
  off=round(math.sin((frame-4)*math.pi/3)*4)*(1 if x<48 else -1) if walk else 0
  ell((x-5,cy+h-4+off,x+6,cy+h+8+off),shade)
 if fam==0:
  ell((cx-w,cy-h,cx+w+6,cy+h),color);ell((cx-w+2,cy-h-11,cx-w+13,cy-h+3),shade);ell((cx+w-10,cy-h-10,cx+w+1,cy-h+4),color)
  ell((cx-8,cy-3,cx+w+11,cy+15),hi);d.rectangle((cx+w,cy+1,cx+w+4,cy+4),fill=DARK)
  if form==3 or form==5:poly([(22,59),(27,47),(34,54),(30,69)],'#c7dd9a');d.line((26,47,27,37),fill='#7e6854',width=3)
  if form==4:ell((31,24,66,36),'#809a58');ell((40,16,56,31),'#aecb76')
  if form==5:d.line([(42,47),(43,72),(56,68)],fill='#426e7d',width=5)
  if form==6:poly([(32,26),(39,9),(46,25),(56,10),(62,30)],'#e8b95d');d.line((27,65,70,65),fill='#de867b',width=4)
 elif fam==1:
  poly([(cx,cy-h-17),(cx-14,cy-h-7),(cx-4,cy-h+2),(cx+8,cy-h-7),(cx+19,cy-h-14)],'#3f805d')
  ell((cx-w,cy-h,cx+w,cy+h),color);d.line((cx,cy-h+5,cx,cy+h-2),fill=hi,width=4)
  poly([(cx-w+3,cy),(cx-w-14,cy-12),(cx-w-14,cy+7),(cx-w,cy+12)],'#87b85c');poly([(cx+w-2,cy),(cx+w+14,cy-12),(cx+w+11,cy+8),(cx+w,cy+12)],'#87b85c')
  if form in [2,4]:
   for a in range(5):
    x=48+int(10*math.cos(a*1.256));yy=18+int(8*math.sin(a*1.256));ell((x-7,yy-6,x+7,yy+6),'#e8a5b9')
   ell((44,14,53,22),'#f4d574')
  if form in [3,5]:poly([(35,34),(23,14),(28,8),(43,30)],'#85694f');poly([(57,32),(67,12),(73,15),(66,40)],'#8fa45c')
  if form==6:
   for x in [23,69]:ell((x-6,30,x+7,44),'#e88776');poly([(x,30),(x-3,23),(x+7,26)],'#5a8c5d')
 elif fam==2:
  poly([(62,61),(83,59),(88,39),(93,56),(85,77),(63,79)],shade)
  ell((cx-w,cy-h,cx+w,cy+h),color);poly([(31,36),(30,22),(38,30),(46,16),(51,31),(63,22),(62,39)],'#e8c77d')
  ell((cx-16,cy+4,cx+15,cy+h),hi)
  if form==4:
   for x,hc in [(26,17),(48,26),(69,17)]:poly([(x-6,30),(x,30-hc),(x+7,29),(x,39)],'#a9d8d7')
  if form==5:poly([(22,40),(8,23),(11,58),(26,65)],'#9b8f9a');poly([(70,41),(84,23),(88,57),(69,67)],'#9b8f9a');d.line((40,57,58,61),fill='#b26346',width=4)
  if form==6:poly([(29,28),(34,10),(43,24),(51,6),(56,25),(65,15),(66,37)],'#efb958');poly([(70,60),(77,47),(82,61),(79,74)],'#fff1ac')
 elif fam==3:
  poly([(26,49),(9,41),(14,57),(9,68),(28,65)],'#a0d9c9');poly([(68,49),(84,41),(79,57),(87,68),(68,66)],'#a0d9c9')
  ell((cx-w,cy-h,cx+w,cy+h),color);ell((cx-13,cy+7,cx+13,cy+h+4),'#c5e8dd');poly([(43,cy-h),(48,cy-h-16),(56,cy-h+1)],hi)
  if form==4:
   for x in [27,65]:d.line([(x,41),(x-3,24),(x-9,18)],fill='#df9498',width=5);d.line((x-3,29,x+5,22),fill='#df9498',width=4)
  if form==5:poly([(18,49),(5,24),(23,31),(31,44)],'#568bba');poly([(69,43),(82,24),(93,29),(79,54)],'#568bba')
  if form==6:
   for x,yy in [(20,24),(73,21),(78,48)]:ell((x-6,yy-6,x+7,yy+7),'#b3e8ea','#5aa6bf');d.point((x-2,yy-2),fill=LIGHT)
 elif fam==4:
  for x,yy,rr in [(29,52,16),(43,40,18),(59,39,16),(70,52,18),(53,61,23)]:ell((x-rr,yy-rr+y,x+rr,yy+rr+y),color)
  d.rectangle((29,48+y,70,65+y),fill=color)
  if form==4:
   for x in [18,77]:poly([(x,57),(x-12,31),(x+3,39),(x+9,53)],'#e6deee')
  if form==5:poly([(36,69),(51,65),(43,79),(55,77),(39,94),(43,81),(30,83)],'#edcb72');d.arc((32,31,67,49),185,340,fill='#666b96',width=5)
  if form==6:
   for i,c in enumerate(['#db8b97','#e8bc80','#a9c784','#83c3d0']):d.arc((19+i*3,9+i*3,80-i*3,64),180,360,fill=c,width=3)
 elif fam==5:
  for x in [20,68]:d.rectangle((x,44+y,x+9,68+y),fill=shade,outline=DARK,width=2);ell((x,45+y,x+9,54+y),'#9eacac')
  d.rounded_rectangle((cx-w,cy-h,cx+w,cy+h),radius=7,fill=color,outline=DARK,width=2)
  d.rectangle((cx-w+5,cy-h+6,cx+w-5,cy+5),fill='#638688',outline=DARK,width=2)
  d.line((48,cy-h,48,cy-h-10),fill=DARK,width=2);ell((44,cy-h-16,52,cy-h-8),'#e7948b')
  if form==4:d.arc((31,55,64,76),0,180,fill='#e8e2c6',width=5);d.rectangle((27,31,34,50),fill='#b1c4b8')
  if form==5:poly([(67,32),(81,21),(77,41),(89,41),(73,65),(77,47),(65,49)],'#a8c9d1');d.rectangle((29,69,68,75),fill='#779fac')
  if form==6:
   for x in [18,75]:ell((x-7,23,x+8,38),'#c68a8d');d.line((x+6,29,x+6,48),fill=DARK,width=3)
   d.rectangle((35,61,60,72),fill='#827caa');d.line((40,63,40,69),fill=LIGHT,width=2);d.line((50,63,50,69),fill=LIGHT,width=2)
 # Stage-specific anatomy and equipment; growth is more than a size change.
 if form==0:
  if fam==0:d.line([(39,30),(42,24),(45,30),(49,24),(52,31)],fill=shade,width=2)
  elif fam==1:d.arc((33,39,63,66),-70,70,fill='#d4df9e',width=3);d.ellipse((41,61,46,65),fill=shade)
  elif fam==2:d.polygon([(61,64),(69,61),(68,69)],fill='#f1d99c',outline=DARK)
  elif fam==3:ell((16,28,24,36),'#d2eded','#80b7c4',1)
  elif fam==4:
   for x,yy in [(30,37),(41,31),(54,31)]:ell((x-4,yy-4,x+4,yy+4),'#e2deec',color,1)
  else:d.line((21,40,13,40),fill=DARK,width=2);d.rectangle((8,35,14,44),fill='#bba5be',outline=DARK)
 elif form==1:
  if fam==0:d.polygon([(33,62),(37,62),(37,69),(33,68)],fill=LIGHT,outline=DARK);d.line((46,29,53,27),fill=hi,width=3)
  elif fam==1:d.line((48,32,48,15),fill='#5a8a5e',width=3);poly([(46,22),(35,17),(34,24),(45,28)],'#a8cb78')
  elif fam==2:
   for x in [29,39,49]:poly([(x,68),(x+4,61),(x+8,68)],'#e6c390')
  elif fam==3:d.arc((28,66,49,87),0,250,fill='#9ad8ca',width=4);d.line((32,70,34,78),fill=DARK,width=2)
  elif fam==4:poly([(29,70),(23,82),(35,77),(40,69)],'#e2ddeb');poly([(62,70),(68,82),(56,77),(53,69)],'#e2ddeb')
  else:d.rectangle((32,64,64,75),fill='#a3b5a9',outline=DARK,width=2);d.line((37,67,43,67),fill=LIGHT,width=2);d.line((51,71,59,71),fill=LIGHT,width=2)
 elif form==2:
  if fam==0:poly([(29,66),(43,69),(38,78),(27,74)],'#8fac7c')
  elif fam==2:ell((24,37,34,50),'#bbce9f');poly([(27,51),(20,58),(31,57)],'#bbce9f')
  elif fam==3:d.arc((28,49,67,76),0,180,fill='#e5d4a8',width=6);d.arc((31,52,64,74),0,180,fill=DARK,width=2)
  elif fam==4:poly([(40,28),(51,8),(65,29)],'#e5d9bd');ell((47,8,55,16),'#e5d9bd')
  elif fam==5:d.arc((23,14,72,41),180,360,fill='#afc2b4',width=9);d.arc((22,13,73,42),180,360,fill=DARK,width=2)
 elif form==3:
  if fam==2:poly([(25,34),(14,20),(15,41),(28,48)],'#dcba85');d.rounded_rectangle((62,49,74,66),3,fill='#9db5ad',outline=DARK,width=2)
  elif fam==3:poly([(36,34),(39,13),(48,26),(55,8),(61,30)],'#99d9d8')
  elif fam==4:poly([(31,27),(55,9),(76,13),(59,20),(77,25),(48,34)],'#dad7e8');d.arc((69,41,91,63),-60,130,fill='#aaa6c7',width=4)
  elif fam==5:d.line((29,32,23,13),fill=DARK,width=3);ell((18,8,27,17),'#c69eb6');d.rounded_rectangle((65,49,79,70),3,fill='#8eada9',outline=DARK,width=2)
 elif form==4 and fam==1:
  for a in range(7):
   x=48+int(14*math.cos(a*math.pi*2/7));yy=18+int(12*math.sin(a*math.pi*2/7));ell((x-6,yy-6,x+6,yy+6),'#e8a5b9')
  ell((43,12,55,24),'#f4d574');poly([(29,70),(14,77),(32,81),(39,73)],'#6e9c63')
 if form==7:
  for x in [31,59]:d.line((x,cy-14,x+9,cy-16),fill='#e9dfcb',width=3)
  d.line([(41,cy+12),(48,cy+26),(57,cy+12)],fill='#e9dfcb',width=4);d.line((76,61,76,87),fill='#88705a',width=3)
 # Eyes and expressive mouth, anchored consistently.
 ey=cy-6;left=39 if fam!=0 else 41;right=57 if fam!=0 else 59
 for x in [left,right]:
  if sleep or frame==28:d.line((x-3,ey+2,x+3,ey+2),fill=DARK,width=2)
  elif happy:d.line([(x-3,ey+2),(x,ey-1),(x+3,ey+2)],fill=DARK,width=2)
  else:d.rectangle((x-2+(1 if frame==29 else -1 if frame==30 else 0),ey-2,x+2,ey+3),fill=DARK);d.point((x-1,ey-1),fill=LIGHT)
 if eat or frame==17:ell((45,cy+5,53,cy+12),DARK)
 elif sad:d.arc((43,cy+8,55,cy+17),185,350,fill=DARK,width=2)
 else:d.arc((43,cy+4,55,cy+12),0,180,fill=DARK,width=2)
 if detail:d.line((29,cy+3,33,cy+8),fill=hi,width=3)
 else:d.rectangle((29,cy+4,32,cy+6),fill=hi)
 if frame in [14,15]:ell((50,73-frame%2*5,66,83-frame%2*5),'#df9b64');d.rectangle((56,70,60,74),fill='#7aa86d')
 if frame in [16,17]:d.rectangle((59,63,70,79),fill='#8ed6e5',outline=DARK,width=2)
 if frame in [20,21]:
  for x,yy in [(29,30),(60,21),(73,44),(18,60)]:ell((x,yy-frame%2*3,x+8,yy+8-frame%2*3),'#e6f4ee','#b0ddd9',1)
 if frame in [22,23]:d.line((17,39,13,46),fill='#c39568',width=2);d.line((78,28,82,35),fill='#c39568',width=2)
 if frame in [24,25]:d.rectangle((38,cy-14,58,cy-9),fill='#e9d7b3');d.line((48,cy-14,48,cy-9),fill='#be6e71',width=2)
 if happy:poly([(76,13+frame%2*4),(80,17+frame%2*4),(76,23+frame%2*4),(71,17+frame%2*4)],'#edacaa')
 if sleep:d.text((73,13-frame%2*3),'z',fill=DARK,font=FONT)
 return im

def ImageColor(s):return tuple(bytes.fromhex(s.lstrip('#')))
def writej(path,data):path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n')
# Form banks. Same geometry maps to four color palettes; detail overlays are composited at runtime.
contact=Image.new('RGB',(8*156,6*166),'#f7efdf');cd=ImageDraw.Draw(contact)
for f,(ident,title,names) in enumerate(FAMILIES):
 forms=[]
 for form in range(9):
  images=[pet(f,form,i) for i in range(32)];file=f'pets/{ident}/{form}.rgb';path=OUT/file;path.parent.mkdir(exist_ok=True);path.write_bytes(b''.join(raw(i) for i in images))
  sheet=Image.new('RGBA',(96*8,96*4));
  for i,im in enumerate(images):sheet.paste(im,((i%8)*96,(i//8)*96))
  if form==0:sheet.save(PRE/f'{ident}-animations.png')
  forms.append({'id':f'{ident}_{form}','name':names[form] if form<8 else 'Ovo','file':file,'width':96,'height':96,'frames':32,'format':'RGB565A8_PLANAR','anchor':[48,81],'animations':ANIMS if form<8 else {'idle':[0,1,2,3],'balancar':[4,5,6,7],'rachadura':[12,13,14,15],'eclosao':[24,25,26,27]}})
  if form<8:contact.paste(images[0].resize((144,144),Image.Resampling.NEAREST),(form*156+6,f*166),images[0].resize((144,144),Image.Resampling.NEAREST));cd.text((form*156+4,f*166+145),names[form],font=FONT,fill=DARK)
 writej(OUT/f'pets/{ident}/family.json',{'id':ident,'name':title,'index':f,'forms':forms,'base_color':COLORS[f],'palettes':[COLORS[f],'#dca986','#91bab3','#b2a6cb'],'details':['pinta','faixa'],'adult_paths':{'tranquilo':4,'explorador':5,'social':6}})
contact.save(PRE/'familias.png')
# Scenes drawn at logical 320x180 then nearest-neighbour scaled by the UI.
scenes=['casa','cozinha','banheiro','banho','quarto','quintal','cuidados','jogos','bercario','album']
def scene(index):
 im=Image.new('RGB',(640,360),'#dce9d9');d=ImageDraw.Draw(im);wall=['#e9ddbd','#e8d7ba','#b8d9d3','#bad9d7','#bfc5d8','#c5e0c9','#d9e6dc','#d2c9db','#e6dfc7','#e3d5ba'][index]
 d.rectangle((0,0,639,245),fill=wall);d.rectangle((0,246,639,359),fill='#c7ad8c');d.rectangle((0,242,639,249),fill='#947c68')
 for x in range(0,640,80):d.line((x,250,x-40,360),fill='#b59c80',width=2)
 for y in [277,315]:d.line((0,y,640,y),fill='#b59c80',width=2)
 def rect(box,color,r=6):d.rounded_rectangle(box,radius=r,fill=color,outline=DARK,width=3)
 # Window has mountains, clouds and a gridded frame.
 rect((55,43,190,161),'#9ec7cc');d.polygon([(58,144),(99,93),(129,124),(158,81),(188,146)],fill='#88af91');d.ellipse((70,56,108,74),fill=LIGHT);d.line((122,46,122,158),fill='#fff0d0',width=5);d.line((58,100,187,100),fill='#fff0d0',width=5);rect((48,158,199,169),'#9b7b61',2)
 # Plant and wall art.
 rect((542,205,592,249),'#c88b70');d.line((566,208,568,149),fill='#6c956d',width=5)
 for x,y in [(548,158),(568,142),(577,172)]:d.ellipse((x-15,y-10,x+16,y+11),fill='#8eaf73',outline='#5c805f',width=2)
 rect((466,48,539,113),'#b48b67');d.rectangle((473,55,532,105),fill='#f1e5bb');d.ellipse((488,66,516,92),fill='#d6aa69')
 if index==0:
  rect((23,209,203,283),'#bd9393');rect((31,202,101,260),'#cda6a3');rect((110,202,186,260),'#cda6a3');d.ellipse((228,269,467,344),fill='#a4b79e',outline='#7b967c',width=4);rect((493,270,527,306),'#e0c176');d.line((498,275,520,299),fill='#c19162',width=4)
 if index==1:
  rect((21,189,202,285),'#b7b69a');d.line((109,193,109,280),fill='#7c8975',width=3);d.rectangle((15,182,208,198),fill='#8a7662');rect((410,210,615,259),'#b7b69a');d.ellipse((415,212,459,229),fill='#88afae');d.line((438,211,438,192),fill=DARK,width=4)
  # table is a separate foreground in runtime, leaving the pet correctly behind it.
 if index in [2,3]:
  for x in range(0,640,40):d.line((x,0,x,244),fill='#9dbab4')
  for y in range(0,244,40):d.line((0,y,640,y),fill='#9dbab4')
  if index==2:rect((63,183,158,268),'#f3f0dc');d.ellipse((59,223,161,271),fill='#f8f5e5',outline=DARK,width=3);d.ellipse((75,231,145,256),fill='#9dbcc2');rect((86,265,137,299),'#e0e2d5')
  else:rect((75,227,202,287),'#eef1df');d.ellipse((81,221,196,248),fill='#a1ccd0');d.line((88,221,88,198),fill=DARK,width=4)
 if index==4:rect((30,240,218,317),'#c9a579');rect((35,217,212,283),'#b1b7d1');rect((35,213,91,248),'#f1e5d3');d.rectangle((24,203,34,324),fill='#8e6e58');d.ellipse((433,27,472,65),fill='#f7e8b4')
 if index==5:
  d.rectangle((0,0,639,250),fill='#b5d9dc');d.ellipse((480,22,531,73),fill='#f4d78d');d.polygon([(0,159),(100,91),(230,169),(340,100),(479,164),(640,81),(640,280),(0,280)],fill='#87b293');d.rectangle((0,241,640,360),fill='#a8c491');d.ellipse((374,247,598,337),fill='#83b6bc',outline='#729f9e',width=4)
  for x in [58,132,570]:d.rectangle((x,130,x+15,256),fill='#9f8464');d.ellipse((x-38,82,x+52,178),fill='#6b9975',outline='#517b60',width=3)
  rect((187,236,266,263),'#bb9870');d.line((197,264,197,292),fill=DARK,width=4);d.line((255,264,255,292),fill=DARK,width=4)
 if index==6:rect((26,210,210,277),'#c3cbaa');rect((435,178,526,267),'#ecedde');d.line((469,194,492,194),fill='#c18580',width=7);d.line((480,184,480,206),fill='#c18580',width=7);rect((39,200,185,229),'#edf1df')
 if index==7:rect((24,217,193,280),'#a79aaf');d.rectangle((41,232,174,242),fill='#e0c893');d.ellipse((70,257,82,267),fill='#edb294');d.ellipse((124,257,136,267),fill='#93c3c0');d.polygon([(464,211),(485,175),(508,211)],fill='#deaaa0');rect((507,236,555,277),'#aec6a9')
 if index==8:rect((65,214,201,281),'#c5aa81');d.line((75,280,75,305),fill=DARK,width=4);d.line((190,280,190,305),fill=DARK,width=4);d.ellipse((71,209,196,239),fill='#f3e6b5');d.line((452,52,452,148),fill='#9c8b71',width=2)
 if index==9:rect((28,204,213,284),'#a58c75');d.line((30,243,210,243),fill=DARK,width=3)
 return im
scenecontact=Image.new('RGB',(640*2,360*5))
for i,n in enumerate(scenes):
 im=scene(i);im.save(PRE/f'{n}.png');(OUT/f'scenes/{n}.rgb').write_bytes(raw(im)[:640*360*2]);scenecontact.paste(im,((i%2)*640,(i//2)*360))
scenecontact.save(PRE/'cenarios.png')
# Decorative sprites and food icons are individual, original drawings.
objects=['Vaso de brotos','Luminária de cristal','Almofada nuvem','Robô de corda','Aquário','Rede pantaneira','Bola','Cata-vento','Livros','Tambor','Cacto','Estrela de madeira']
for i,name in enumerate(objects):
 im=Image.new('RGBA',(64,64));d=ImageDraw.Draw(im);c=COLORS[i%6]
 if i%6==0:d.polygon([(19,37),(45,37),(40,59),(24,59)],fill='#c89175',outline=DARK);d.line((32,38,32,12),fill='#5e8e65',width=3);d.ellipse((15,12,32,29),fill=c);d.ellipse((31,7,48,24),fill='#81b67d')
 elif i%6==1:d.polygon([(12,46),(26,13),(37,7),(52,47),(42,57),(20,57)],fill=c,outline=DARK);d.line((27,16,30,50),fill=LIGHT,width=3)
 elif i%6==2:d.rounded_rectangle((10,20,54,52),10,fill=c,outline=DARK,width=2);d.line((15,25,48,46),fill=LIGHT,width=3)
 elif i%6==3:d.rectangle((18,20,46,50),fill=c,outline=DARK,width=2);d.ellipse((24,28,29,33),fill=DARK);d.ellipse((36,28,41,33),fill=DARK);d.line((32,20,32,10),fill=DARK,width=2)
 elif i%6==4:d.rounded_rectangle((8,15,56,55),8,fill='#8fcbd0',outline=DARK,width=2);d.ellipse((21,29,40,42),fill=c);d.polygon([(39,34),(47,28),(47,43)],fill=c)
 else:d.line((8,23,8,60),fill='#967c61',width=3);d.line((55,23,55,60),fill='#967c61',width=3);d.arc((6,10,57,53),0,180,fill=c,width=7)
 # Each object has a distinct ornament rather than only recoloring.
 if i>=6:
  im=Image.new('RGBA',(64,64));d=ImageDraw.Draw(im)
  if i==6:
   d.ellipse((10,10,54,54),fill='#d98c7d',outline=DARK,width=2);d.arc((15,8,48,54),60,275,fill='#efd594',width=6);d.arc((8,17,56,46),10,170,fill='#8bbbc0',width=4)
  elif i==7:
   d.line((31,23,31,60),fill='#927257',width=4)
   for pts,c in [([(31,26),(13,8),(13,31)],'#df9e93'),([(31,26),(52,8),(30,8)],'#b3cb91'),([(31,26),(52,47),(52,24)],'#8cbcc9'),([(31,26),(13,47),(33,47)],'#e5c473')]:d.polygon(pts,fill=c,outline=DARK)
   d.ellipse((27,22,36,31),fill=LIGHT,outline=DARK)
  elif i==8:
   for j,c in enumerate(['#8cae99','#ca9d9c','#a7adca']):
    y=44-j*13;d.rounded_rectangle((9+j*3,y,54-j*2,y+12),2,fill=c,outline=DARK,width=2);d.rectangle((17+j*3,y+3,49-j*2,y+8),fill=LIGHT)
  elif i==9:
   d.rectangle((13,23,52,52),fill='#cda071',outline=DARK,width=2);d.ellipse((13,15,52,31),fill='#ece4c8',outline=DARK,width=2);d.arc((13,41,52,60),0,180,fill=DARK,width=2)
   for x in range(17,51,8):d.line([(x,29),(x+4,49),(x+8,29)],fill='#827c77',width=2)
   d.line((6,8,29,20),fill=DARK,width=2);d.ellipse((4,6,11,13),fill='#dca58d')
  elif i==10:
   d.rounded_rectangle((27,7,39,48),5,fill='#7da773',outline=DARK,width=2);d.line([(31,28),(18,28),(18,19)],fill='#7da773',width=8);d.line([(36,36),(48,36),(48,23)],fill='#7da773',width=8);d.polygon([(18,43),(47,43),(43,60),(23,60)],fill='#ce9678',outline=DARK)
   for y in [15,25,35]:d.line((32,y,35,y),fill=LIGHT,width=2)
  else:
   pts=[(32+int((25 if j%2==0 else 11)*math.sin(j*math.pi/5)),32-int((25 if j%2==0 else 11)*math.cos(j*math.pi/5))) for j in range(10)];d.polygon(pts,fill='#d6b27a',outline=DARK);d.line((20,37,44,26),fill='#b48d60',width=2);d.line((23,43,43,33),fill='#b48d60',width=2)
 (OUT/f'ui/decor{i}.rgb').write_bytes(raw(im));im.save(PRE/f'decor{i}.png')
foods=[('arroz','Arroz com legumes',0,24,4,3),('sopa','Sopa do quintal',0,22,4,4),('graos','Prato de grãos',0,25,3,3),('peixe','Peixinho do lago',0,26,4,2),('salada','Salada colorida',0,20,3,4),('milho','Milho cozido',0,22,4,2),('maca','Maçã',1,14,4,3),('banana','Banana',1,15,4,2),('melancia','Melancia',1,12,5,3),('manga','Manga',1,13,5,3),('biscoito','Biscoito caseiro',2,6,9,-1),('bolinho','Bolinho de fruta',2,8,9,-1)]
for i,(ident,name,kind,full,happy,health) in enumerate(foods):
 im=Image.new('RGBA',(64,64));d=ImageDraw.Draw(im);d.ellipse((4,19,60,56),fill='#e9e4cf',outline=DARK,width=2)
 if i==0:
  d.ellipse((13,24,51,47),fill='#ece1b9')
  for j in range(18):x=18+(j*7)%29;y=27+(j*11)%16;d.line((x,y,x+3,y),fill='#fff2d3',width=2)
  for x,y in [(19,35),(35,27),(42,37)]:d.rectangle((x,y,x+5,y+5),fill='#91b47a')
 elif i==1:
  d.ellipse((8,16,56,50),fill='#c58e69',outline=DARK,width=2);d.ellipse((11,15,53,40),fill='#dfbf78',outline=DARK,width=2)
  for x,y in [(17,23),(34,20),(41,29)]:d.ellipse((x,y,x+6,y+5),fill='#89a678')
 elif i==2:
  for j in range(20):x=15+(j*13)%35;y=24+(j*7)%19;d.ellipse((x,y,x+5,y+4),fill='#c39d67',outline='#947958')
 elif i==3:
  d.ellipse((13,24,47,43),fill='#9ebfc2',outline=DARK,width=2);d.polygon([(44,33),(56,24),(56,44)],fill='#87aab7',outline=DARK);d.ellipse((20,29,24,33),fill=DARK);d.line((31,26,31,40),fill='#6f989d',width=2)
 elif i==4:
  for j in range(7):x=14+(j*9)%32;y=23+(j*7)%17;d.ellipse((x,y,x+12,y+8),fill=['#a2bf81','#7c9e71','#e5c884'][j%3],outline='#608463')
  for x,y in [(21,28),(37,35)]:d.ellipse((x,y,x+9,y+8),fill='#d88d7d',outline='#a76a65')
 elif i==5:
  d.rounded_rectangle((22,11,42,49),7,fill='#e0bd69',outline=DARK,width=2)
  for x in [26,32,38]:
   for y in range(17,44,7):d.ellipse((x-2,y-2,x+2,y+2),fill='#f3d78c')
  d.polygon([(22,42),(13,25),(18,52),(31,54)],fill='#86a370',outline=DARK)
 elif i==6:
  d.ellipse((16,15,47,46),fill='#d58675',outline=DARK,width=2);d.line((32,19,35,8),fill='#7a6956',width=3);d.ellipse((35,8,47,15),fill='#88aa73')
 elif i==7:
  d.arc((10,4,51,43),0,180,fill='#e5c56c',width=13);d.arc((12,4,49,41),0,180,fill='#f3d98a',width=5);d.rectangle((10,19,14,24),fill='#8c7958')
 elif i==8:
  d.polygon([(12,17),(53,17),(32,48)],fill='#8fb18a',outline=DARK);d.polygon([(15,19),(49,19),(32,42)],fill='#dc9286');
  for x,y in [(25,24),(37,24),(31,33)]:d.line((x,y,x+1,y+3),fill=DARK,width=2)
 elif i==9:
  d.ellipse((20,11,45,48),fill='#e4ad70',outline=DARK,width=2);d.line((34,15,38,8),fill='#7b7958',width=3);d.ellipse((37,6,49,14),fill='#8bab77')
 elif i==10:
  d.ellipse((16,13,49,47),fill='#c99b6a',outline=DARK,width=2)
  for x,y in [(25,23),(39,28),(29,37)]:d.ellipse((x,y,x+4,y+4),fill='#81675b')
 else:
  d.polygon([(18,30),(48,30),(43,52),(23,52)],fill='#b99270',outline=DARK);d.ellipse((16,17,50,38),fill='#e1b4a3',outline=DARK,width=2);d.ellipse((22,11,43,27),fill='#eed0b3',outline=DARK);d.ellipse((29,7,36,14),fill='#c87b74')
 (OUT/f'ui/food{i}.rgb').write_bytes(raw(im))
writej(OUT/'data/foods.json',[{'id':x[0],'name':x[1],'short_name':['Arroz','Sopa','Grãos','Peixe','Salada','Milho','Maçã','Banana','Melancia','Manga','Biscoito','Bolinho'][i],'kind':x[2],'fullness':x[3],'happiness':x[4],'health':x[5],'icon':i} for i,x in enumerate(foods)])
writej(OUT/'data/decor.json',[{'id':i,'name':n,'sprite':f'ui/decor{i}.rgb','unlock':'convivência e jogos'} for i,n in enumerate(objects)])
writej(OUT/'data/rules.json',{'incubation_min':180,'incubation_max':300,'baby_ms':7200000,'child_ms':86400000,'teen_ms':259200000,'adult_ms':1209600000,'decay_per_10_minutes':[1.8,1.1,1.0,1.2,3.2],'offline_tolerance_seconds':7200,'offline_limit_seconds':43200,'checkpoint_seconds':60,'classic_risk_hours':48})
writej(OUT/'data/games.json',[{'id':i+1,'name':n,'instruction':t} for i,(n,t) in enumerate([('Jokenpô','Escolha pedra, papel ou tesoura. Vence quem fizer 3 pontos.'),('Par ou ímpar','Escolha paridade e um número de 0 a 5. São 5 rodadas. A soma zero é par.'),('Esquerda ou direita','Adivinhe para onde seu amigo vai olhar. São 5 tentativas.'),('Maior ou menor','O próximo número será maior ou menor? Igual repete, sem penalidade.'),('Memória','Encontre os pares de retratos. Toque em duas cartas por vez.'),('Sequência musical','Observe cores e símbolos. Depois repita a sequência.'),('Pega-frutinha','Arraste a cesta e apanhe as frutas durante 30 segundos.'),('Esconde-esconde','Encontre seu amigo em um dos seis objetos. Três rodadas.')])])
# Bitmap font file: fixed A8 cells, 32-bit Unicode table and advance widths.
fnt=ImageFont.truetype(str(fontpath),26);codes=list(range(32,256))+[0x2022,0x2019,0x2014,0x2190,0x2192,0x2605]
header=struct.pack('<4I',0x31464742,len(codes),36,40);table=b'';bitmap=b''
for cp in codes:
 cell=Image.new('L',(36,40));d=ImageDraw.Draw(cell);d.text((0,0),chr(cp),font=fnt,fill=255);adv=math.ceil(fnt.getlength(chr(cp)));table+=struct.pack('<II',cp,adv);bitmap+=cell.tobytes()
(OUT/'ui/font.bin').write_bytes(header+table+bitmap)
fnt=ImageFont.truetype(str(fontpath),22);header=struct.pack('<4I',0x31464742,len(codes),28,34);table=b'';bitmap=b''
for cp in codes:
 cell=Image.new('L',(28,34));d=ImageDraw.Draw(cell);d.text((0,0),chr(cp),font=fnt,fill=255);table+=struct.pack('<II',cp,math.ceil(fnt.getlength(chr(cp))));bitmap+=cell.tobytes()
(OUT/'ui/font-small.bin').write_bytes(header+table+bitmap)
fruit=Image.new('RGBA',(32,32));d=ImageDraw.Draw(fruit);d.ellipse((4,8,27,29),fill='#d98b78',outline=DARK,width=2);d.line((15,11,18,3),fill='#82694f',width=2);d.ellipse((18,2,28,9),fill='#86ad73');d.line((8,14,9,19),fill='#f1b59b',width=2);(OUT/'ui/fruit.rgb').write_bytes(raw(fruit))
basket=Image.new('RGBA',(96,48));d=ImageDraw.Draw(basket);d.arc((13,0,83,44),180,360,fill='#7e654e',width=5);d.polygon([(9,20),(87,20),(78,45),(19,45)],fill='#b58d61',outline=DARK);d.line((10,21,86,21),fill='#dfbf83',width=5)
for x in range(21,83,12):d.line((x,24,x-2,43),fill='#dfbd80',width=3)
for y in [28,36]:d.line((15,y,82,y),fill='#8e6e51',width=2)
(OUT/'ui/basket.rgb').write_bytes(raw(basket))

# Pixel-art care symbols, stored on the SD with the rest of the UI.
for i in range(8):
 im=Image.new('RGBA',(48,48));d=ImageDraw.Draw(im)
 if i==0:
  d.ellipse((9,11,38,39),fill='#e9e4cf',outline=DARK,width=2);d.ellipse((15,17,32,33),fill='#b5c88c');d.line((4,11,4,37),fill=DARK,width=2);d.line((42,11,42,37),fill=DARK,width=2)
 elif i==1:
  d.rounded_rectangle((6,23,43,39),6,fill='#a6d0d4',outline=DARK,width=2);d.arc((9,6,25,27),180,360,fill=DARK,width=3);d.line((9,14,9,27),fill=DARK,width=2)
  for x,y in [(24,12),(32,8),(36,18)]:d.ellipse((x,y,x+7,y+7),fill='#eff4e5',outline='#82afb7')
 elif i==2:
  d.ellipse((9,6,39,37),fill='#eed397',outline=DARK,width=2);d.ellipse((22,2,45,27),fill='#f6eedc');d.polygon([(8,7),(10,2),(13,7),(18,9),(13,11),(11,16),(8,11),(3,9)],fill='#c9b6cf')
 elif i==3:
  d.ellipse((7,10,25,29),fill='#da9694',outline=DARK,width=2);d.ellipse((22,10,40,29),fill='#da9694',outline=DARK,width=2);d.polygon([(8,22),(39,22),(24,41)],fill='#da9694');d.line([(7,23),(24,42),(40,23)],fill=DARK,width=2)
 elif i==4:
  d.rounded_rectangle((9,12,39,39),4,fill='#e9e9d7',outline=DARK,width=2);d.line((18,12,18,7,30,7,30,12),fill=DARK,width=2);d.rectangle((21,18,27,33),fill='#bf8585');d.rectangle((16,22,32,28),fill='#bf8585')
 elif i==5:
  d.rounded_rectangle((7,11,39,38),5,fill='#b8c9c4',outline=DARK,width=2);d.line((14,19,14,30),fill=DARK,width=3);d.line((9,25,20,25),fill=DARK,width=3);d.ellipse((27,19,32,24),fill='#d49b91');d.ellipse((32,26,37,31),fill='#e1c478')
 elif i==6:
  d.line((24,35,24,8),fill='#788d60',width=3);d.ellipse((6,10,24,26),fill='#b1c58b',outline=DARK,width=2);d.ellipse((24,5,42,21),fill='#8cae79',outline=DARK,width=2);d.line((14,20,24,29,33,14),fill='#6e865c',width=2)
 else:
  d.rounded_rectangle((6,12,42,36),3,fill='#d9c49b',outline=DARK,width=2);d.line((24,13,24,36),fill=DARK,width=2)
  for y in [20,26,31]:d.line((10,y,19,y),fill='#8f7965',width=2);d.line((28,y,38,y),fill='#8f7965',width=2)
 (OUT/f'ui/action{i}.rgb').write_bytes(raw(im))

# Synthesized original motifs. Continuous 24s cycles with envelopes at each note.
RATE=22050
def wav(path,data):
 data=np.clip(data,-.8,.8);pcm=np.round(data*32767).astype('<i2');
 with wave.open(str(path),'wb') as w:w.setnchannels(1);w.setsampwidth(2);w.setframerate(RATE);w.writeframes(pcm.tobytes())
def tone(freq,duration,amp=.2):
 t=np.arange(round(RATE*duration))/RATE;env=np.minimum(1,t/.015)*np.minimum(1,(duration-t)/.06);return amp*env*(np.sin(2*np.pi*freq*t)+.18*np.sin(4*np.pi*freq*t))
sfx=['botao','confirmar','cancelar','recusa','atencao','comida','agua','banheiro','descarga','banho','sono','eclosao','evolucao','tratamento','vitoria','derrota','carinho','explorar','nota0','nota1','nota2','nota3']
for i,name in enumerate(sfx):
 notes=[220*2**(((i*3+j*4)%24)/12) for j in range(3 if name in ['eclosao','evolucao','vitoria'] else 2)]
 wav(OUT/f'audio/sfx/{name}.wav',np.concatenate([tone(f,.11,.18) for f in notes]))
for fam,(ident,_,_) in enumerate(FAMILIES):
 for j,name in enumerate(['chamado','alegria','sono']):wav(OUT/f'audio/sfx/{ident}_{name}.wav',np.concatenate([tone(200+fam*43+k*60,.12 if j!=2 else .22,.16) for k in ([0,2,1] if j==0 else [0,2,4] if j==1 else [3,2,0])]))
for i,name in enumerate(['casa','bercario','cozinha','banheiro','quarto','quintal','cuidados','jogos']):
 data=np.zeros(RATE*24);scale=[0,2,4,7,9,12,9,7];root=146.832*2**((i%5)/12)
 for j in range(48):
  start=int(j*.5*RATE);note=scale[(j*(i%3+1)+i)%len(scale)];seg=tone(root*2**(note/12),.48,.10 if name!='quarto' else .05);data[start:start+len(seg)]+=seg
  if j%4==0:
   bass=tone(root/2,1.9,.045);data[start:start+len(bass)]+=bass
 # Fade endpoints to zero: no discontinuity in loop.
 data[:441]*=np.linspace(0,1,441);data[-441:]*=np.linspace(1,0,441);wav(OUT/f'audio/music/{name}.wav',data)
writej(OUT/'ui/strings.json',{'title':'Bichinhos do Tab5','language':'pt-BR'})

# Inherited adult signatures remain visible on the distinct elder body.
elder_contact=Image.new('RGB',(6*144,3*144),'#f7efdf')
for f,(ident,_,_) in enumerate(FAMILIES):
 for adult in range(3):
  im=Image.new('RGBA',(96,96));d=ImageDraw.Draw(im);c=COLORS[f]
  if f==0:
   if adult==0:d.ellipse((31,22,66,34),fill='#809a58',outline=DARK,width=2);d.ellipse((40,14,56,29),fill='#aecb76',outline=DARK,width=2)
   elif adult==1:d.rounded_rectangle((20,49,31,68),3,fill='#6d9ca9',outline=DARK,width=2)
   else:d.polygon([(32,26),(39,9),(46,25),(56,10),(62,30)],fill='#e8b95d',outline=DARK)
  elif f==1:
   if adult==0:
    for a in range(5):
     x=48+int(10*math.cos(a*1.256));y=18+int(8*math.sin(a*1.256));d.ellipse((x-7,y-6,x+7,y+6),fill='#e8a5b9',outline=DARK)
    d.ellipse((44,14,53,22),fill='#f4d574')
   elif adult==1:d.polygon([(27,36),(21,14),(28,8),(43,30)],fill='#85694f',outline=DARK)
   else:
    for x in [23,69]:d.ellipse((x-6,30,x+7,44),fill='#e88776',outline=DARK)
  elif f==2:
   if adult==0:
    for x,h in [(26,17),(48,26),(69,17)]:d.polygon([(x-6,30),(x,30-h),(x+7,29),(x,39)],fill='#a9d8d7',outline=DARK)
   elif adult==1:d.polygon([(22,40),(8,23),(11,58),(26,65)],fill='#9b8f9a',outline=DARK)
   else:d.polygon([(33,28),(41,7),(49,26),(60,9),(66,32)],fill='#efb958',outline=DARK)
  elif f==3:
   if adult==0:
    for x in [27,65]:d.line([(x,41),(x-3,24),(x-9,18)],fill='#df9498',width=5)
   elif adult==1:d.polygon([(18,49),(5,24),(23,31),(31,44)],fill='#568bba',outline=DARK)
   else:
    for x,y in [(20,24),(73,21),(78,48)]:d.ellipse((x-6,y-6,x+7,y+7),fill='#b3e8ea',outline='#5aa6bf',width=2)
  elif f==4:
   if adult==0:d.polygon([(18,57),(6,31),(21,39),(27,53)],fill='#e6deee',outline=DARK)
   elif adult==1:d.polygon([(36,69),(51,65),(43,79),(55,77),(39,94),(43,81),(30,83)],fill='#edcb72',outline=DARK)
   else:
    for i,c in enumerate(['#db8b97','#e8bc80','#a9c784','#83c3d0']):d.arc((19+i*3,9+i*3,80-i*3,64),180,360,fill=c,width=3)
  else:
   if adult==0:d.arc((31,55,64,76),0,180,fill='#e8e2c6',width=5)
   elif adult==1:d.polygon([(67,32),(81,21),(77,41),(89,41),(73,65),(77,47),(65,49)],fill='#a8c9d1',outline=DARK)
   else:
    for x in [18,75]:d.ellipse((x-7,23,x+8,38),fill='#c68a8d',outline=DARK)
  (OUT/f'pets/{ident}/elder{adult}.rgb').write_bytes(raw(im))
  preview=pet(f,7,0);preview.alpha_composite(im);elder_contact.paste(preview.resize((144,144),Image.Resampling.NEAREST),(f*144,adult*144),preview.resize((144,144),Image.Resampling.NEAREST))
elder_contact.save(PRE/'idosos-heranca.png')

(OUT/'licenses').mkdir(exist_ok=True)
shutil.copyfile(ROOT/'assets_src/fonts/LICENSE-DejaVu.txt',OUT/'licenses/DejaVu.txt')
shutil.copyfile(ROOT/'LICENSE-MIT',OUT/'licenses/original-MIT.txt')
writej(OUT/'manifest.json',{'schema_version':1,'content_version':'0.1.0','engine_api_version':1,'families':[f[0] for f in FAMILIES],'scenes':scenes,'files':[{'path':str(p.relative_to(OUT)),'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in sorted(OUT.rglob('*')) if p.is_file() and p.name!='manifest.json' and 'saves' not in p.parts and 'album' not in p.parts]})
(OUT/'saves/LEIA-ME.txt').write_text('Progresso criado pelo jogo. Atualizações preservam esta pasta.\n')
(OUT/'album/LEIA-ME.txt').write_text('Retratos e backups pessoais. Não distribuímos progresso.\n')
print('Generated',len(list(OUT.rglob('*'))),'entries;',sum(p.stat().st_size for p in OUT.rglob('*') if p.is_file()),'bytes')
