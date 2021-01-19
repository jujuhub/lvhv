/*
 * HighVoltage.h
 *
 * purpose
 *    header for HighVoltage.c
 */

#define NCHAR 10
#define HV_MAX 4000.0
#define DAC_VMAX 5.0
#define DAC_VREF 2.048

/*
 * function:    checkHV
 * description: checks the status of the high voltage
 *
 * parameters:  none
 * return:      none
 */
void checkHV(void);

/*
 * function:    enableHV
 * description: enables/disables high voltage
 *
 * parameters:  pwrEn = on(1) or off(0)
 * return:      none
 */
 void enableHV(int pwrEn);

/*
 * function:    setHV
 * description: sets the maximum high voltage (photocathode)
 *
 * parameters:  hvSet = max high voltage to set
 * return:      none
 */
void setHV(float vset);
