# basic script to control power relay R Pi expansion module

import time
import RPi.GPIO as GPIO

# relay GPIO #
RLY1 = 26 # left
RLY2 = 20 # middle
RLY3 = 21 # right

# pin config
GPIO.setmode(GPIO.BCM) # use Broadcom pin-numbering scheme
GPIO.setup(RLY1, GPIO.OUT)
GPIO.setup(RLY2, GPIO.OUT)
GPIO.setup(RLY3, GPIO.OUT)

interval = 1

# turn off relay
GPIO.output(RLY1, GPIO.LOW)

i = 0
while (i < 5):
	GPIO.output(RLY1, GPIO.HIGH)
	time.sleep(interval)
	GPIO.output(RLY1, GPIO.LOW)
	time.sleep(interval)
	i += 1

# turn off relay
GPIO.output(RLY1, GPIO.LOW)
GPIO.cleanup()
