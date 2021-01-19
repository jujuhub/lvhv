/*
 * TriggerBoard.c
 *
 * purpose
 *    set and read trigger threshold for LAPPD trigger board
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>

#include "TriggerBoard.h"
#include "canlib.h"
#include "lib.h"

int main(void)
{
  printf("*---------*---------*---------*---------*---------*\n");
  printf("*---- This program controls the trigger board ----*\n");
  printf("*---------*---------*---------*---------*---------*\n");
  printf("...\n Press Ctrl+C to Quit \n...\n");

  /* check that the CAN bus network and low voltage are up */
  char user_input[NCHAR];
  bool canbusEnabled = false;

  while (!canbusEnabled)
  {
    printf("\nAre the CAN bus network and low voltage up? [Y/N] > ");
    fgets(user_input, NCHAR, stdin);
    if (!isValidInput(user_input))
    {
      printf("!!! Please enter a valid input\n");
      continue;
    }

    if (checkYN(user_input) != 0 && checkYN(user_input) != 1)
    {
      printf("  Oopsies! Try again :)\n");
      continue;
    }

    if (checkYN(user_input) == 1)
    {
      printf("  ok\n");
      canbusEnabled = true;
    }
    else
    {
      printf("Go bring the CAN network up!\n");
      printf("  Command: sudo /sbin/ip link set can0 up type can bitrate 250000\n");
      printf("\nAnd the low voltage lines!\n");
      printf("  Command: ./LowVoltage\n");
      printf("\nQuitting program...\n");
      return 0;
    }
  } // end while loop

  /* check the low voltage status */
  checkTrigger(0);
  checkTrigger(1);

  bool userDone = false;
  while (!userDone)
  {
    char user_input[NCHAR];
    char *tmp = NULL;
    long long userOption, dac_i;
    float setTrig = 0.;

    printf("\nWhat would you like to do?\n");
    printf("  [1] Set trigger threshold\n");
    printf("  [2] Check trigger threshold\n");
    printf("  [3] Nothing else\n");
    fgets(user_input, NCHAR, stdin);
    userOption = strtol(user_input, &tmp, NCHAR);
    if (userOption < 1 || userOption > 3)
    {
      printf("Try again!\n");
      continue;
    }
    else
    {
      printf(" >>> You chose option [%lld] <<<\n\n", userOption);
    }

    switch(userOption)
    {
      case 1:
        printf("Which trigger (DAC0 or DAC1) would you like to set? [0/1] > ");
        fgets(user_input, NCHAR, stdin);
        dac_i = strtol(user_input, &tmp, NCHAR);
        if (dac_i != 0 && dac_i != 1)
        {
          printf("Oopsies!\n");
          break;
        }
        setTrigger(dac_i, setTrig);
        checkTrigger(dac_i);
        break;

      case 2:
        checkTrigger(dac_i);
        break;

      case 3:
        userDone = true;
        printf("\nQuitting program.. \n\n");
        break;

      default :
        printf("What...?\n");
        break;
    } // end switch statement
  } // end while loop
  return 0;
}

void checkTrigger(int dac_i)
{
  printf("\nChecking trigger threshold of DAC %d\n", dac_i);

  // send CAN msg to request trigger settings
  char *can_msg[] = {"dummy", "can0", "0BC#0000FEEDBEEF0000"};
  char *can_msg2[] = {"dummy", "can0", "0EF#0000BEEFFEED0000"};

  if (dac_i == 0)
  {
    cansend(can_msg);
  }

  if (dac_i == 1)
  {
    cansend(can_msg2);
  }

  delay(100);
  // read the sent-back msg and print results
  printf("  The trigger on DAC %d is ...\n\n", dac_i);
  // ...
}

void setTrigger(int dac_i, float setTrig)
{
  if (dac_i) // turn ON low voltage
  {
    char *can_msg[] = {"dummy", "can0", "020#DEADBEEF00000000"};
    printf("Turning LV >>> ON <<<\n");
//    cansend(can_msg);
  }
  if (!dac_i) // turn OFF low voltage
  {
    char *can_msg[] = {"dummy", "can0", "010#00000000DEADBEEF"};
    printf("Turning LV >>> OFF <<<\n");
//    cansend(can_msg);
  }

  delay(100);
}
