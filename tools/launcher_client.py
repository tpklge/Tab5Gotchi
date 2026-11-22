"""Minimal local launcher console client, without resetting USB on open."""
import json,time
import serial
class Tab5:
    def __init__(self,port,timeout=30):
        self.timeout=timeout
        self.serial=serial.Serial(port=None,baudrate=115200,timeout=.2,write_timeout=5)
        self.serial.rts=True;self.serial.dtr=True;self.serial.port=port
        self.serial.open();self.serial.rts=False;self.serial.dtr=False
        self.serial.write(b'\r\n');time.sleep(.3);self.serial.reset_input_buffer()
    def close(self):self.serial.close()
    def command(self,*arguments):
        for value in arguments:
            if any(ord(c)<32 for c in str(value)):raise ValueError('Control character')
        quoted=['"'+str(value).replace('\\','\\\\').replace('"','\\"')+'"' for value in arguments]
        self.serial.write(('tab5 '+' '.join(quoted)+'\r\n').encode())
        deadline=time.monotonic()+self.timeout
        while time.monotonic()<deadline:
            line=self.serial.readline().decode(errors='replace');marker=line.find('TAB5 {')
            if marker<0:continue
            try:reply=json.loads(line[marker+5:])
            except json.JSONDecodeError:continue
            if reply.get('error'):raise RuntimeError(reply)
            return reply
        raise TimeoutError('Launcher não respondeu; confira operação em andamento')
