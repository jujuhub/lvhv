import numpy as np
import subprocess

command = 'sudo /sbin/ip link set can0 up type can bitrate 250000'

user_input = raw_input("Please enter Y or N: ")
print "Is ", user_input, " your final answer? "
