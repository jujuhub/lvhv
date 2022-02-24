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
    printf("\nAre the CAN bus network AND low voltage up? [Y/N] > ");
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

  /* check trigger board status */
  checkTrigger(DAC0);
  checkTrigger(DAC1);

  bool userDone = false;
  while (!userDone)
  {
    char user_input[NCHAR];
    char *tmp = NULL;
    long long userOption, dac_i = 0;
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
        printf("Which trigger (DAC0/DAC1) would you like to set? [0/1] > ");
        fgets(user_input, NCHAR, stdin);
        dac_i = strtol(user_input, &tmp, NCHAR);
        if (dac_i != 0 && dac_i != 1)
        {
          printf("Oopsies!\n");
          break;
        }
        printf("What threshold would you like to set for this trigger? [0., %0.1f] V > ", VREF);
        fgets(user_input, NCHAR, stdin);
        setTrig = (float)atof(user_input);
        if (setTrig > VREF || setTrig < 0.)
        {
          printf("\n  The value entered is out of bounds (0. < trg < %.1f). Please try again.\n", VREF);
          break;
        }
        printf("\nYou entered >> %f V <<\n", setTrig);
        setTrigger(dac_i, setTrig);
        checkTrigger(dac_i);
        break;

      case 2:
        printf("Which trigger (DAC0/DAC1) would you like to check? > ");
        fgets(user_input, NCHAR, stdin);
        dac_i = strtol(user_input, &tmp, NCHAR);
        if (dac_i != 0 && dac_i != 1)
        {
          printf("Oopsies!\n");
          break;
        }
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

  char msg[] = "000#0000000000000000";
  if (dac_i == DAC0)
  {
    // set msg ID = 0x0BC
    msg[1] = 'B';
    msg[2] = 'C';
    printf("DEBUG:  msg = %s (DAC%d)\n", msg, dac_i);
  }
  if (dac_i == DAC1)
  {
    // set msg ID = 0x0EF
    msg[1] = 'E';
    msg[2] = 'F';
    printf("DEBUG: msg = %s (DAC%d)\n", msg, dac_i);
  }

  // send CAN msg to request trigger settings
  int rcvStat = -8;
  char rcv_msg[21];
  struct SlowControlsData sc;

  char *can_msg[] = {"dummy", "can0", msg};
  cansend(can_msg);
  usleep(USLP+50);
  rcvStat = canread(rcv_msg);
  if (rcvStat == 1)
  {
    decodeCANmsg(&sc, rcv_msg);
    if (dac_i == 0)
    {
      printf(" > trigger bd DAC%d: %.3f V\n", dac_i, sc.trig_dac0);
    }
    if (dac_i == 1)
    {
      printf(" > trigger bd DAC%d: %.3f V\n", dac_i, sc.trig_dac1);
    }
  }
  else { printf(" @@@ CAN message receive error code: %d\n", rcvStat); }
  delay(3*MSEC);
  rcv_msg[0] = '\0';

}

int setTrigger(int dac_i, float setTrig)
{
  if (setTrig > VREF) { setTrig = VREF; }

  char msg[] = "000#0000000000000000";
  char tmp[17];
  if (dac_i == DAC0)
  {
    // set msg ID = 0x0AB
    msg[1] = 'A';
    msg[2] = 'B';
    printf("DEBUG:  msg = %s (DAC%d)\n", msg, dac_i);
  }
  if (dac_i == DAC1)
  {
    // set msg ID = 0x0DE
    msg[1] = 'D';
    msg[2] = 'E';
    printf("DEBUG: msg = %s (DAC%d)\n", msg, dac_i);
  }

  // convert voltage to hex code
  int k = 0;
  k = (int)(setTrig * 4095 / VREF);
  printf("DEBUG:  k = %d, %X (hex)\n", k, k);
  sprintf(tmp, "%X", k);
  printf("DEBUG:  tmp = %s, len = %d\n", tmp, strlen(tmp));
  if (strlen(tmp) > 3)
  {
    printf("  !!! Unable to set voltage. Please try again.\n");
    return 1;
  }

  int c = 0;
  int delta = 3-strlen(tmp);
  while (c < strlen(tmp))
  {
    msg[4+c+delta] = tmp[c];
    printf("DEBUG:  msg = %s\n", msg);
    c++;
  }

  // prepare and send CAN msg
  char *can_msg[] = { "dummy", "can0", msg};
  cansend(can_msg);
  delay(1000);

  return 0;
}
