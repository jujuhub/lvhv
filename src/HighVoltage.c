/*
 * HighVoltage.c
 *
 * purpose
 *    enables/disables the high voltage line on LVHV board
 *    sets high voltage value
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>

#include "HighVoltage.h"
#include "canlib.h"
#include "lib.h"

int main(void)
{
  printf("*---------*---------*---------*---------*---------*\n");
  printf("*---- This program controls the high voltage  ----*\n");
  printf("*---------*---------*---------*---------*---------*\n");
  printf("...\n Press Ctrl+C to Quit \n...\n");

  /* check that the CAN bus network is up */
  char user_input[NCHAR];
  bool canbusEnabled = false;

  while (!canbusEnabled)
  {
    printf("\nAre the CAN bus network and LV up? [Y/N] > ");
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

  /* check the high voltage status */
  checkHV();

  bool userDone = false;
  while (!userDone)
  {
    char user_input[NCHAR];
    char *tmp = NULL;
    float vset = 0.;
    long long userOption, pwrEn;

    printf("\nWhat would you like to do?\n");
    printf("  [1] Enable/Disable high voltage (on/off)\n");
    printf("  [2] Set high voltage\n");
    printf("  [3] Check status of high voltage\n");
    printf("  [4] Nothing else\n");
    fgets(user_input, NCHAR, stdin);
    userOption = strtol(user_input, &tmp, NCHAR);
    if (userOption < 1 || userOption > 4)
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
        printf("Do you want to enable(1) or disable(0) the high voltage? > ");
        fgets(user_input, NCHAR, stdin);
        pwrEn = strtol(user_input, &tmp, NCHAR);
        if (pwrEn != 0 && pwrEn != 1)
        {
          printf("Oopsies!\n");
          break;
        }
        enableHV(pwrEn);
        checkHV();
        break;

      case 2:
        printf("What is the max (PC) high voltage you want to set? > ");
        fgets(user_input, NCHAR, stdin);
        // convert decimal/float, and then to HEX
        vset = (float)atof(user_input);
//        printf("DEBUG: testing trigger board reading...\n");
        setHV(vset);
        break;

      case 3:
        checkHV();
        break;

      case 4:
        userDone = true;
        printf("Quitting program.. \n\n");
        break;

       default :
        printf("What...?\n");
        break;
    } // end switch statement
  } // end while loop
  return 0;
}

void checkHV(void)
{
  printf("\nChecking status of high voltage lines...\n");
  // send CAN msg to request low voltage status
  char *can_msg[] = {"dummy", "can0", "042#0000BEEFDEAD0000"};
  cansend(can_msg);
  delay(100);
  // read the sent-back msg and print results
  printf("  The high voltage is ON / OFF\n\n");
  // ...
}

void enableHV(int pwrEn)
{
  if (pwrEn) // ENABLE high voltage
  {
    char *can_msg[] = {"dummy", "can0", "040#BEEFDEAD00000000"};
    printf("HV is >>> ENABLED <<<\n");
    cansend(can_msg);
  }
  if (!pwrEn) // DISABLE high voltage
  {
    char *can_msg[] = {"dummy", "can0", "030#00000000BEEFDEAD"};
    printf("HV is >>> DISABLED <<<\n");
    cansend(can_msg);
  }

  delay(100);
}

void setHV(float vset)
{
  float vpct = vset / HV_MAX;
  printf("  fraction of max HV output (4kV) = %f\n", vpct);
  float dac_vout = vpct * DAC_VMAX;
  printf("  DAC output voltage = %f\n", dac_vout);
  int k = 0;
  k = (int)(pow(2,12) * dac_vout / DAC_VREF);
  printf("  k = %X\n  k << 3 = %X\n", k, (k<<3));

  char msg[] = "060#";
//  sprintf(msg, "%X\n", k<<3);
  char ch = '0';
  printf("  msg = %s, len = %d\n", msg, strlen(msg));

  while (1)
  {
    if (strlen(msg) > 17) break;
    strncat(msg, &ch, 1);
//    printf("  msg = %s, len = %d\n", msg, strlen(msg));
  }

  // convert decimal to binary
  // convert binary to hexadecimal
  // create a can_msg and then send
}

/* FOR TRIGGER BOARD
void setHV(void)
{
//  printf("Setting max high voltage to:   %f\n", hvSet);
  // create a string based on hvSet
//  char *can_msg[] = {"dummy", "can0", "050#0000BEEF0000DEAD"};
  char *can_msg[] = {"dummy", "can0", "0AB#07FE000000000000"};
  cansend(can_msg);
  delay(1000);

  char *can_msg_rcv[] = {"dummy", "can0", "0BC#0000FEED0000BEEF"};
  cansend(can_msg_rcv);
  delay(100);
}
*/
