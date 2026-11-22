#!/usr/bin/env python3
"""Measure a timed game while draining serial continuously, without captures."""
import json,time,serial
from pathlib import Path
s=serial.Serial(port=None,baudrate=115200,timeout=.2,write_timeout=5);s.rts=True;s.dtr=True;s.port='/dev/cu.usbmodem12201';s.open();s.rts=False;s.dtr=False;time.sleep(.3);s.reset_input_buffer()
records=[]
def command(value,key):
 s.write(('gotchi '+value+'\n').encode());deadline=time.monotonic()+30
 while time.monotonic()<deadline:
  line=s.readline().decode(errors='replace').strip()
  if line.startswith('GOTCHI '):
   result=json.loads(line[7:])
   if key in result:return result
 raise TimeoutError(value)
try:
 initial=command('status','profile');assert initial['ready'] and not initial['fault']
 if initial['screenOff']:command('tap 40 600','tap');time.sleep(.3)
 command('click 306','clicked');start=time.monotonic();next_query=start+5
 while time.monotonic()-start<29:
  line=s.readline().decode(errors='replace').strip()
  if line.startswith('METRICS ') or line.startswith('TELEMETRY '):records.append({'line':line,'seconds':time.monotonic()-start})
  if time.monotonic()>=next_query:
   r=command('status','profile');records.append({'state':r,'seconds':time.monotonic()-start})
   print(json.dumps({'seconds':round(time.monotonic()-start,1),'game':r['game'],'active':r['gameActive'],'elapsedFruit':r['fruitY'],'fps':r['fps'],'uiHz':r.get('uiHz'),'audioErrors':r['audioErrors']}),flush=True)
   assert r['id']==initial['id'] and not r['fault'];next_query+=5
 command('click 43','clicked');command('click 1','clicked')
finally:
 s.close();Path('private/hardware/final-benchmark.json').write_text(json.dumps(records,indent=2)+'\n')
