/*
 * TriggerBoard.h
 *
 * purpose
 *    header for TriggerBoard.c
 */

#define NCHAR 10
#define DAC0 0
#define DAC1 1
#define TRIG_MAX 3.0 // V
#define VREF 2.981 //ideally 3.0V

/*
 * function:    checkTrigger
 * description: checks the trigger threshold of DACi
 *
 * parameters:  dac_i = DAC0(0) or DAC1(1)
 * return:      none
 */
void checkTrigger(int dac_i);

/*
 * function:    setTrigger
 * description: sets the trigger threshold of DACi
 *
 * parameters:  dac_i = DAC0(0) or DAC1(1)
 * return:      (int)
 */
int setTrigger(int dac_i, float setTrig);
