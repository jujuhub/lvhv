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
#include <time.h>
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

  /* read previously set voltage from file if starting up */
  float vprev = 0.;
  vprev = readPrevHV(PREV_HV_FILE);

  /* check the high voltage status */
  if (!checkHV())
  {
    printf(" test!\n");
    vprev = 0.;
  }

  time_t tstamp;
  tstamp = time(NULL);

  bool userDone = false;
  while (!userDone)
  {
    FILE *fp = fopen(PREV_HV_FILE, "a+");
    char user_input[NCHAR];
    char *tmp = NULL;
    float vset = 0., vdiff = 0., vtmp = 0., vread = 0.;
    float sign = -1.0;
    long long userOption, pwrEn;

    printf("\nWhat would you like to do?\n");
    printf("  [1] Enable/Disable high voltage (on/off)\n");
    printf("  [2] Set high voltage\n");
    printf("  [3] Check status of high voltage\n");
    printf("  [4] Check previously set voltage\n");
    printf("  [5] Nothing else\n");
    fgets(user_input, NCHAR, stdin);
    userOption = strtol(user_input, &tmp, NCHAR);
    if (userOption < 1 || userOption > 5)
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
        printf("PREVIOUS VOLTAGE = %f V\n\n", vprev);
        printf("What is the max (PC) high voltage you want to set? [0., 3276.] > ");
        fgets(user_input, NCHAR, stdin);
        vset = (float)atof(user_input);
        if (vset > HV_MAX || vset < 0.)
        {
          printf("\n  !!! The voltage you input is out of bounds (0. < HV < 3276.). Please try again.\n");
          break;
        }
        printf("\nYou input >> %f V <<\n", vset);
        if (vset > vprev)
        {
          sign = 1.0;
        }
        vtmp = vprev;
        vdiff = fabs(vset - vtmp);
        printf("DEBUG:  [initial] vtmp = %f,  vdiff = %f\n", vtmp, vdiff);
        while (vtmp != vset)
        {
          if (vdiff < DV) // check if close to final voltage
          {
            vtmp += sign*vdiff;
          }
          else // increment by DV volts
          {
            vtmp += sign*DV;
          }
          vdiff = fabs(vset - vtmp);
          printf("DEBUG:  [updated] vtmp = %f, vdiff = %f\n", vtmp, vdiff);
          setHV(vtmp);
        }
        // write vset to file
        vprev = vset;
        tstamp = time(NULL);
        fprintf(fp, "%f V %s", vprev, asctime(localtime(&tstamp)));
        break;

      case 3:
        checkHV();
        break;

      case 4:
        vread = readPrevHV(PREV_HV_FILE);
        printf("PREVIOUSLY SET HV: %f V\n", vread);
        break;

      case 5:
        userDone = true;
        printf("Quitting program.. \n\n");
        break;

       default :
        printf("What...?\n");
        break;
    } // end switch statement

    fclose(fp);
  } // end while loop

  return 0;
}

bool checkHV(void)
{
  bool HV_EN = true;

  printf("\nChecking status of high voltage lines...\n");
  char *can_msg[] = {"dummy", "can0", "042#0000BEEFDEAD0000"};
  cansend(can_msg);
  delay(100);

  // read the sent-back msg and print results
  printf("  The high voltage is ON / OFF\n\n");
  // ...
  //if (ON)
  //  HV_EN = true;
  return HV_EN;
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

int setHV(float vset)
{
  if (vset > HV_MAX)
  {
    vset = HV_MAX;
  }

  float vpct = vset / C40N_MAX;
  printf("  fraction of max HV output (4kV) = %f\n", vpct);
  float dac_vout = vpct * DAC_VMAX;
  printf("  DAC output voltage = %f\n", dac_vout);

  // convert into DAC input code
  int k = 0;
  k = (int)(pow(2,12) * dac_vout / DAC_VREF);
  printf("DEBUG:  k = %d,  k = %X (hex),  k << 3 = %X (hex)\n", k, k, (k<<3));

  // prepare CAN msg
  char msg[] = "050#0000000000000000";
  char tmp[17];
  sprintf(tmp, "%X\n", (k<<3));
//  printf("DEBUG:  msg = %s, len = %d\n", msg, strlen(msg));
  printf("DEBUG:  tmp = %s, len = %d\n", tmp, strlen(tmp));
  if (strlen(tmp) > 5)
  {
    printf("  !!! Unable to set voltage. Please try again.\n");
    return 1;
  }

  int c = 0;
  int delta = 5-strlen(tmp);
  while (c < strlen(tmp)-1)
  {
    msg[4+c+delta] = tmp[c];
//    printf("DEBUG:  msg = %s, len = %d\n", msg, strlen(msg));
    c++;
  }

  // create a can_msg and then send
  char *can_msg[] = {"dummy", "can0", msg};
  printf("DEBUG:  can_msg[2] = %s\n", can_msg[2]);
//  cansend(can_msg);
  delay(3000);

  return 0;
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
