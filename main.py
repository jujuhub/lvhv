### purpose: controls the main routine of the LVHV board

import numpy as np
import subprocess

### one-time functions

# setup CAN bus connection (separate script?); maybe do this on start up..

### POWER UP THE LVHV BOARD (SWITCH POWER RELAY)
PWR_EN = False

while (not PWR_EN):
  user_input = raw_input("Do you wish to power up (+12V) the LVHV board? (Y/N) > ")
  if (not user_input.isalpha()):
    print "Yo, please enter a valid input. "
  user_input = user_input.upper()
  print "Your answer > ", user_input

  if (user_input[0] == 'Y'):
    # switch relay ON
    PWR_EN = True
    print "The LVHV is powered up. You can now communicate with it. "

  elif (user_input[0] == 'N'):
    print "OK :( \nYou cannot do anything until the LVHV board is powered up. "
    # wait for user to say YES

  else:
    print "Try again! :) "


### TURN ON LOW VOLTAGE LINES
LV_EN = False

while (not LV_EN):
  user_input = raw_input("Do you wish to turn ON the low voltage lines? (Y/N) > ")
  if not user_input.isalpha():
    print "Please enter a valid input homie. "
  user_input = user_input.upper()
  print "Your FINAL answer > ", user_input

  if (user_input[0] == 'Y'):
    # send CAN msg to turn on LV lines
    LV_EN = True # may want to do a logic check to confirm that it's ON
    print "The low voltage lines are ON. \nYou can now (1) read humidity and temperature, \n(2) set the high voltage, (3) set the trigger \nthreshold. "

  elif (user_input[0] == 'N'):
    print "OK :( \nYou can't do much else here... "
    # wait for user to say YES

  else: 
    print "Try harder. "

### INITIAL STATUS CHECKS
INIT_STAT = False

while (not INIT_STAT):
  user_input = raw_input("Do you wish to perform initial checks on the System? (Y/N) > ")
  if (not user_input.isalpha()):
    print "MY GUY. PLEASE enter a VALID input. "
  user_input = user_input.upper()
  print "Your answer > ", user_input

  if (user_input[0] == 'Y'):
    print "Fetching data... "
    # fetch RH&T data, light level data, previous voltage values (stored in a text file?)
    # print info and store to another text file
    print "Data fetched! "

    INIT_STAT = True

  if (user_input[0] == 'N'):
    print "OK... Your loss. If something breaks, it's your fault... "
    while (user_input[0] != 'Y'):
      user_input = raw_input("Are you SURE? (Y/N) > ")
      if (not user_input.isalpha()):
        print "Valid input please! "
      user_input = user_input.upper()

      if (user_input[0] == 'Y'):
        print "OK. I hope you know what you're doing... "
        INIT_STAT = True
        break
      if (user_input[0] == 'N'):
        print "I'm glad you changed your mind! Fetching data now... "
        # fetch RH&T data, light level data, previous voltage values (stored in a text file?)
        # print info and store to another text file
        print "Data fetched! "


# write a function to get initial checks? and return a status value? 

# send CAN message to request for RH&T, light levels, etc.
#if (RHT == 'OK'):
	# proceed with routine
#if (RHT != 'OK'):
print ("WARNING!! RH&T levels are above threshold! Do you want to continue? (Y/N) > ")
	# wait for user input
	# keep reading RH&T until levels are OK; maybe put this whole thing
	#	in a loop

# after status checks come out OK
print ("Do you wish to turn on the HV? (Y/N) > ")
# wait for user input
#if (user_input == 'YES' or 'Y'):
	# start HV turn on sequence.. call on a script?
	# set HV_ON_OFF logic to be high, but first set DAC output to be 0V
#if (user_input == 'NO' or 'N'):
	# wait until user says yes
	# or move on to checking RH&T

while(True):
  print "Fetching RH&T data... \n"
	# send CAN message to request RH&T
	# receive CAN message with RH&T data
	# convert to human-readable format
	# save in a .txt file: data status ('Normal' or 'Stale')
	#			RH in decimal value
	#			T in decimal value
	#			data type (from R Pi script or user command)
	# keep a running average of RH & T (separate script?)

	# probe for user input?

	# delays? 

#end of script
