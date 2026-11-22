#!/usr/bin/env python3
"""Exercise the firmware on the physical Tab5 over USB. No simulation shortcut."""
import argparse,json,time
from pathlib import Path
import serial
p=argparse.ArgumentParser();p.add_argument('--port',default='/dev/cu.usbmodem12201');p.add_argument('commands',nargs='*',default=['status']);p.add_argument('--capture',type=Path);a=p.parse_args()
s=serial.Serial(port=None,baudrate=115200,timeout=.5,write_timeout=5);s.rts=True;s.dtr=True;s.port=a.port;s.open();s.rts=False;s.dtr=False;time.sleep(.2);s.reset_input_buffer()
def command(value):
 s.write(('gotchi '+value+'\n').encode());deadline=time.monotonic()+30
 while time.monotonic()<deadline:
  line=s.readline().decode(errors='replace').strip()
  if line.startswith('GOTCHI '):
   reply=json.loads(line[7:]);expected={'status':'profile','click':'clicked','tap':'tap','save':'saved','screenshot':'screenshot'}.get(value.split()[0])
   if expected is None or expected in reply:return reply
  if line:print(line,flush=True)
 raise TimeoutError(value)
try:
 for c in a.commands:print(json.dumps(command(c),ensure_ascii=False));time.sleep(.25)
 if a.capture:
  s.write(b'gotchi capture\n');deadline=time.monotonic()+30
  while time.monotonic()<deadline:
   line=s.readline()
   if line.startswith(b'GOTCHI_BINARY '):break
  else:raise TimeoutError('capture')
  n=int(line.split()[1]);data=bytearray();deadline=time.monotonic()+60
  while len(data)<n and time.monotonic()<deadline:data.extend(s.read(min(n-len(data),65536)))
  if len(data)!=n:raise IOError(f'truncated capture {len(data)}/{n}')
  a.capture.parent.mkdir(parents=True,exist_ok=True);a.capture.write_bytes(data);print('Captured',n,'bytes to',a.capture)
finally:s.close()
