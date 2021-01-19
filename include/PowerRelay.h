/*
 * PowerRelay.h
 *
 * purpose
 *    Header for PowerRelay.c
 */

#define RLY1 25
#define RLY2 28
#define RLY3 29
#define RLYDLY 500 // ms; delay
#define MAX_NRLY 3
#define NCHAR 10
#define NOPTIONS 4

/*
 * function:    initRelay
 * description: initializes the power relay HAT
 *
 * parameters:  none
 * return:      none
 */
void initRelay(void);

/*
 * function:    powerRelay
 * description: switches a relay on/off
 *
 * parameters:  rlyN = relay to switch on/off (N = 1,2,3)
 *              rlyPWR = on(1) or off(0)
 * return:      (bool) if successful switching
 */
bool powerRelay(int rlyN, int rlyPWR);

/*
 * function:    checkRelay
 * description: checks status of each relay
 *
 * parameters:  none
 * return:      none
 */
void checkRelay(void);
