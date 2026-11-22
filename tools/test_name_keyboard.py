#!/usr/bin/env python3
"""Exercise the same HID-to-name handler as the A164, without changing the saved name."""
import json,time,serial
from pathlib import Path
s=serial.Serial(port=None,baudrate=115200,timeout=.2,write_timeout=5);s.rts=True;s.dtr=True;s.port='/dev/cu.usbmodem12201';s.open();s.rts=False;s.dtr=False;time.sleep(.3);s.reset_input_buffer();records=[]
def cmd(value):
 key={'status':'profile','click':'clicked','key':'key','tap':'tap'}[value.split()[0]]
 s.write(('gotchi '+value+'\n').encode());deadline=time.monotonic()+30
 while time.monotonic()<deadline:
  line=s.readline().decode(errors='replace').strip()
  if line.startswith('GOTCHI '):
   r=json.loads(line[7:])
   if key in r:records.append({'command':value,'reply':r});time.sleep(.05);return r
 raise TimeoutError(value)
def click(code):cmd('click '+str(code));time.sleep(.2)
def key(usage,mod=0):cmd(f'key {mod} {usage}')
try:
 before=cmd('status');assert before['ready'] and not before['fault'];original=before['name'];identity=before['id']
 if before['screenOff']:cmd('tap 40 600');time.sleep(.3)
 click(23);assert cmd('status')['page']=='name'
 key(4,1) # Ctrl+A clears the field
 for usage in [23,8,22,23,8]:key(usage)
 assert cmd('status')['nameDraft']=='teste'
 key(0x4a);key(0x4c);assert cmd('status')['nameDraft']=='este'
 key(0x4f);key(0x2a);key(0x4d);key(30,2)
 assert cmd('status')['nameDraft']=='ste!'
 click(34);assert cmd('status')['nameDraft']=='ste!'
 click(34);assert cmd('status')['nameDraft']=='ste!'
 key(0x29);assert cmd('status')['name']==original
 click(23);key(0x28);assert cmd('status')['page']=='info' and cmd('status')['name']==original
 click(23);key(4,1);key(0x28);assert cmd('status')['page']=='name'
 key(0x29);click(23)
 final=cmd('status');assert final['id']==identity and final['name']==original and not final['fault']
 print('PASS edição HID, símbolos, cursor, reconstrução, Enter e Esc; nome original preservado')
 print('Teclado físico detectado:',bool(final['keyboardConnected']))
finally:
 s.close();Path('private/hardware/keyboard-name-tests.json').write_text(json.dumps(records,ensure_ascii=False,indent=2)+'\n')
