# lvhv

This software is used to manage the slow controls of the LAPPD system. The main interface is between the Raspberry Pi in the breakout box and the microcontroller on the LVHV board. The user can send a message from the Pi to the LVHV microcontroller requesting for information (such as relative humidity and temperature), and in turn, the microcontroller will communicate with the relevant peripherals. 

## Programs
### PowerRelay
This program controls the +12V power required to power the LVHV board, and thus the ACDC boards and LAPPD themselves. The user communicates with the power relay HAT that sits atop the Raspberry Pi. 

### LowVoltage
This program controls the low voltage lines. 
