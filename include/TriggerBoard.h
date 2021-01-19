/*
 * LowVoltage.h
 *
 * purpose
 *    header for LowVoltage.c
 */

#define NCHAR 10

/*
 * function:    checkLV
 * description: checks the status of the low voltage
 *
 * parameters:  dac_i = DAC0(0) or DAC1(1)
 * return:      none
 */
void checkTrigger(int dac_i);

/*
 * function:    powerLV
 * description: turns low voltage lines on/off
 *
 * parameters:  pwrEn = on(1) or off(0)
 * return:      none
 */
 void setTrigger(int dac_i, float setTrig);
