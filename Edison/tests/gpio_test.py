import mraa
import time

x = mraa.Gpio(32)
x.dir(mraa.DIR_IN)

while True:
  print x.read()
  time.sleep(0.1)
