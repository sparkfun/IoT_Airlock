import serial

ser = serial.Serial(port='/dev/ttyMFD1', baudrate=9600)
ser.write("Hello")

while 1:
  if ( ser.inWaiting() > 0 ):
    c = ser.read()
    print c

