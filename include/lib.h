/*
 * lib.h
 *
 * purpose
 *    header for lib.c
 */

#define RLY1 25
#define RLY2 28
#define RLY3 29
#define RLYDLY 500 // relay delay
#define LOW 0
#define HIGH 1
#define OPSDLY 1000 // delay between measurements

/* checks if user input is alpha valid */
bool isValidInput(char *usr_inp);

/* checks yes or no */
int checkYN(char *usr_inp);

/*
 * function:    fetchRHT
 * description: makes measurement of relative humidity and temperature
 *
 * parameters: none
 * return: none
 */
void fetchRHT(void);

/*
 * function:    statCheck
 * description: makes initial status checks (RH&T, etc.)
 *
 * parameters:  none
 * return:      none
 */
void statCheck(void);

/*
 * function:    readPrevHV
 * description: reads last line of a .txt file and extracts
 *              previously set high/trigger voltage value
 *
 * parameters:  file name
 * return:      (float) previously set HV value
 */
float readPrevV(char *fname);

/**************************************************
 *                NOT USED ANYMORE                *
 **************************************************/

/*
 * function:    initSequence
 * description: initial routine sequence (power up LVHV board
 *              and low voltage lines)
 *
 * parameters:  none
 * return:      none
 */
void initSequence(void);

/*
 * function:    initRelayHAT
 * description: initialize the power relay HAT
 *
 * parameters:  none
 * return:      none
 */
void initRelayHAT(void);

/*
 * function:    relayHAT
 * description: turn on/off power relay HAT
 *
 * parameters:  rlyN = relay channel to switch (1/2/3);
 *                     0 = all chs
 *              rlyEN = enable(1) / disable(0) relay
 *
 * return:      none
 */
void relayHAT(int rlyN, int rlyEN);
