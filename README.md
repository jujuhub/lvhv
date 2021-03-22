# lvhv

This software is used to manage the slow controls of the LAPPD system. The main interface is between the Raspberry Pi in the breakout box and the microcontroller on the LVHV board. The user can send a message from the Pi to the LVHV microcontroller requesting for information (such as relative humidity and temperature), and in turn, the microcontroller will communicate with the relevant peripherals. 

The following programs should be executed in the order that they are listed. 

## Programs
### PowerRelay
This program controls the +12V power required to power the LVHV board, and thus the ACDC boards and LAPPD themselves. The user communicates with the power relay HAT that sits atop the Raspberry Pi. The user needs to run this program first before moving on to the programs below. 

When executed, the user has several options to choose from:

  (1) Switch a relay (1/2/3) on (1) or off (0).
  
  (2) Switch ALL relays on (1) or off (0).
  
  (3) Check the status (on/off) of all relays.
  
  (4) Nothing else. (Quit the program.)


### LowVoltage
This program controls the low voltage lines (+3.3V, +2.5V, +1.2V). The low voltage lines power up the ACDC boards. In addition, the I2C communication bus is pulled up to the +3.3V line so that the trigger board can be added to the I2C communication bus. This program must be run after the PowerRelay program 

When the program is executed, the menu options are:

  (1) Turn the low voltage on (1) or off (0). 
  
  (2) Check the status (on or off) of low voltage lines and read the voltage values (ADC). 
  
  (3) Nothing else. (Quit the program.)



### TriggerBoard
This program manages the trigger board thresholds for LAPPD data. There are 2 DACs on the trigger board: DAC0 sets the threshold for individual LAPPD channels, while DAC1 sets the threshold for the sum of the channels (multiplicity comparator). This program can be run at any time, as long as the PowerRelay and LowVoltage programs have been carried out. 

When this program is executed, the user can: 

  (1) Set the trigger threshold of DAC0 or DAC1 in volts (V). 
  
  (2) Check the trigger threshold. 
  
  (3) Nothing else. (Quit the program.)


### SlowControl
This program manages the slow controls of the LAPPD system. Users can check the relative humidity and temperature, photodiode light levels, and low voltage values. If any of the values go out of the expected range (e.g. exceed a threshold value), shifters will be alerted and an emergency shutdown of the LAPPD can be performed manually or automatically. 

Once the program is executed, it will run indefinitely unless interrupted by the user. It can be called after running the HighVoltage program. 


### HighVoltage
This program controls the high voltage (HV) that powers the LAPPD. There are two separate steps: the HV must be enabled, then the HV can be set. This needs to happen in that order, otherwise the LAPPD may be damaged if the HV is set before enabling it. 

Here are the options: 

  (1) Enable (1)/Disable (0) high voltage. (THIS NEEDS TO HAPPEN BEFORE SETTING THE HV VALUE.)
  
  (2) Set high voltage (in V). 
  
  (3) Check the status (on or off) of the HV. (Unfortunately, we are unable to read back the exact voltage from the module.) 
  
  (4) Check previously set voltage. (Read back the voltage that you input.) 
  
  (5) Nothing else. (Quit the program.)


## Questions
If you have any questions about this software, please contact Julie He (juhe@ucdavis.edu).
