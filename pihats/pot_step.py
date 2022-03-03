import RPi.GPIO as GPIO
import datetime
import time

DISCHARGE_TIME = 0.005  #ms
READ_TIME = 1  #s
SAVEFILE = 'res_time_data.txt'

GPIO.setmode(GPIO.BCM)

a_pin = 23
b_pin = 24

def discharge():
  GPIO.setup(a_pin, GPIO.IN)
  GPIO.setup(b_pin, GPIO.OUT)
  GPIO.output(b_pin, False)
  time.sleep(DISCHARGE_TIME)

def charge_time():
  GPIO.setup(b_pin, GPIO.IN)
  GPIO.setup(a_pin, GPIO.OUT)
  count = 0
  GPIO.output(a_pin, True)
  while not GPIO.input(b_pin):
    count += 1
  return count

def analog_read():
  discharge()
  return charge_time()

f = open(SAVEFILE, 'a+')

while True:
  tstamp = datetime.datetime.now().strftime("%Y-%m-%d,%H:%M:%S")
  measurement = analog_read()
  print(str(measurement) + "," + tstamp)
  f.write(str(measurement) + "," + tstamp + "\n")
  time.sleep(READ_TIME)
