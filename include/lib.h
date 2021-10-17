/*
 * lib.h
 *
 * purpose
 *    header for lib.c
 */

#include <ctype.h>
#include <errno.h>
#include <glob.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <wiringPi.h>

#define RLY1 25
#define RLY2 28
#define RLY3 29
#define RLYDLY 500 // relay delay
#define LOW 0
#define HIGH 1
#define OPSDLY 1000 // delay between measurements

struct SlowControlsData {
  double hum, temp;
  bool lv_en;
  double lvA, lvB, lvC;
  bool hv_en;
  double hv;
  double photodiode;
  double trig_dac0, trig_dac1;
};

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
float readPrevHV(char *fname);

int decodeCANmsg(struct SlowControlsData *sc, char *rcv_msg);
void decodeRHT(struct SlowControlsData *sc, char *canmsg);
void decodeLV(struct SlowControlsData *sc, char *canmsg);
void decodeHV(struct SlowControlsData *sc, char *canmsg);
void decodePhotodiode(struct SlowControlsData *sc, char *canmsg);
void decodeTrigBd(struct SlowControlsData *sc, char *canmsg);

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

