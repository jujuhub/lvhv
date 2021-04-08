/*
 * SlowControl.c
 *
 * written by: J. He
 *
 * purpose
 *    this manages the slow controls routine of the LVHV board
 *      - fetches RH&T data
 * usage
 *    user needs to execute the program and let it run
 *      ./SlowControl
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>

#include "lib.h"
#include "canlib.h"

int main(void)
{
  printf("*---------*---------*---------*---------*---------*\n");
  printf("*- This program fetches RH&T, light levels, etc. -*\n");
  printf("*---------*---------*---------*---------*---------*\n");
  printf("...\n Press Ctrl+C to Quit \n...\n\n");

  /*** check that LVHV board is powered up with +12V ***/
  char user_input[10];
  bool lvhvPowered = false;

  while (!lvhvPowered)
  {
    printf("Did you turn on the LVHV board first? [Y/N] > ");
    fgets(user_input, 10, stdin);
    if (!isValidInput(user_input))
    {
      printf("!!! Please enter a valid answer \n");
      continue;
    }

    if (checkYN(user_input) != 0 && checkYN(user_input) != 1)
    {
      printf("  Oopsies! Try again :)\n");
      continue;
    }

    if (checkYN(user_input) == 1)
    {
      printf("  Ok \n");
      lvhvPowered = true;
    }
    else
    {
      printf("  Go turn it on then! Run this command: ./PowerRelay\n");
      printf("Quitting program...\n");
      return 0;
    }
  }

  /*** check that the low voltage is on ***/
  bool lvEnabled = false;

  while (!lvEnabled)
  {
    printf("Is the low voltage on? [Y/N] > ");
    fgets(user_input, 10, stdin);
    if (!isValidInput(user_input))
    {
      printf("!!! Please enter a valid response \n");
      continue;
    }

    if (checkYN(user_input) != 0 && checkYN(user_input) != 1)
    {
      printf("  Oopsies! Try again :)\n");
      continue;
    }

    if (checkYN(user_input) == 1)
    {
      lvEnabled = true;
      printf("  ok \n");
    }
    else
    {
      printf("  Go turn on the LV! Command: ./LowVoltage \n");
      printf("Can't continue with this program. Quitting..\n");
      return 0;
    }
  }

  /* INITIAL STATUS CHECKS */
//  statCheck();

// write a function to get initial check? and return status value?

// send CAN message to request for RH&T, light levels, etc.
// if (RHT == 'OK')
//   proceed with routine
// if (RHT != 'OK')
  printf("WARNING!! RH&T levels are above threshold! Do you want to continue? (Y/N)> ");
  // wait for user input
  // keep reading RH&T until levels are OK?

// after status checks come out OK
  printf("Do you wish to turn on the HV? (Y/N)> ");
// wait for user input
// if YES
//    start HV turn on sequence.. call on a script?
//    set HV_ON_OFF logic to be HIGH, but first set DAC output to be 0V
// if NO
//    wait until user says yes
//    or move on to checking RH&T

  while (1)
  {
    printf("Fetching RH&T data...\n");
    // send CAN msg to request RH&T
    char *can_msg_rht[] = {"dummy", "can0", "123#0000000000000000"};
    cansend(can_msg_rht);
//    delay(100); // need this delay to read data
//    canread();
    delay(3000);

    // receive CAN msg with RH&T data
    // convert to human-readable format
    // save in .txt file: data status ('Normal' or 'Stale')
    //                    RH in decimal
    //                    T in decimal
    //                    data type (from RPi script or user command?)
    // keep running average of RH&T (separate script?)

    // probe for user input?


    // send CAN msg to request photodiode levels
    printf("Fetching light levels...\n");
    char *can_msg_pd[] = {"dummy", "can0", "00D#000D000D000D000D"};
    cansend(can_msg_pd);
    delay(3000);

    // send CAN msg to request low voltage outputs
    printf("Fetching low voltage values...\n");
    char *can_msg_lv[] = {"dummy", "can0", "3AD#00AD00AD00AD00AD"};
    cansend(can_msg_lv);
    delay(3000);

    // send CAN msg to request trigger board thresholds
    printf("Fetching trigger board threshold voltages...\n");
    char *can_msg_dac0[] = {"dummy", "can0", "0BC#0000000000000000"}; 
    cansend(can_msg_dac0);
    delay(3000);

    char *can_msg_dac1[] = {"dummy", "can0", "0EF#0000000000000000"};
    cansend(can_msg_dac1);
    delay(48000);

  } // end main loop

  return 0;
}

/* ******************************
 * functions to test
 * ******************************/
