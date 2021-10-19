/*
 * LowVoltage.h
 *
 * purpose
 *    header for LowVoltage.c
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>

#define NCHAR 10

/*
 * function:    checkLV
 * description: checks the status of the low voltage
 *
 * parameters:  none
 * return:      none
 */
void checkLV(void);

/*
 * function:    powerLV
 * description: turns low voltage lines on/off
 *
 * parameters:  pwrEn = on(1) or off(0)
 * return:      none
 */
 void powerLV(int pwrEn);
