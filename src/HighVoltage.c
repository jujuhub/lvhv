/*
 * HighVoltage.c
 *
 * purpose
 *    enables/disables the high voltage line on LVHV board
 *    sets high voltage value
 */

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
  printf("\n >> last HV setting: \n");
  vprev = readPrevHV(PREV_HV_FILE);

  struct SlowControlsData sc;

  /* check the high voltage status */
  if (!checkHV(&sc))
  {
    //printf(" test!\n"); //DEBUG
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
        checkHV(&sc);
        break;

      case 2:
        // need to make sure HV is enabled

        vread = readPrevHV(PREV_HV_FILE);
        if (!checkHV(&sc))
        {
          printf("\n @@@ WARNING !! HV is DISABLED!\n\n"); 
          vread = vprev;
        }
        printf(" > PREVIOUSLY SET HV: %.2f V\n\n", vread);
        //printf(" > PREVIOUSLY SET HV = %.2f V\n\n", vprev);
        printf("What is the max (PC) high voltage you want to set? [0., %.2f] > ", HV_MAX);
        fgets(user_input, NCHAR, stdin);
        vset = (float)atof(user_input);
        if (vset > HV_MAX || vset < 0.)
        {
          printf("\n  !!! The voltage you input is out of bounds (0. < HV < %.2f). Please try again.\n", HV_MAX);
          break;
        }
        printf("\nYou input >> %.2f V <<\n", vset);
        if (vset > vprev) { sign = 1.0; }
        vtmp = vprev;
        vdiff = fabs(vset - vtmp);
        printf("  [initial] vtmp = %0.2f V,  vdiff = %.2f V\n\n", vtmp, vdiff);
        while (vtmp != vset)
        {
          //check if close to final voltage
          if (vdiff < DV) { vtmp += sign*vdiff; }
          //increment by DV volts
          else { vtmp += sign*DV; }

          vdiff = fabs(vset - vtmp);
          printf("  [updated] vtmp = %.2f V, vdiff = %.2f V\n", vtmp, vdiff);
          setHV(vtmp);
        }
        // write vset to file
        vprev = vset;
        tstamp = time(NULL);
        fprintf(fp, "%.2f V %s", vprev, asctime(localtime(&tstamp)));
        break;

      case 3:
        checkHV(&sc);
        break;

      case 4:
        vread = readPrevHV(PREV_HV_FILE);
        printf(" > PREVIOUSLY SET HV: %0.2f V\n", vread);
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

/*
bool checkHV(struct SlowControlsData sc)
{
  FILE *fp = fopen(PREV_HV_FILE, "a+");
  time_t tstamp;
  tstamp = time(NULL);

  bool HV_EN = false;
  int rcvStat = -8;
  char rcv_msg[21];
  struct SlowControlsData sc;

  printf("\nChecking status of high voltage lines...\n");
  char *can_msg[] = {"dummy", "can0", "034#0000000000000000"};
  cansend(can_msg);
  //delay(100);
  usleep(USLP);
  rcvStat = canread(rcv_msg);
  if (rcvStat == 1)
  {
    decodeCANmsg(&sc, rcv_msg);
    HV_EN = sc.hv_en;
    printf("  The high voltage is >>> ");
    printf(HV_EN ? "ENABLED (ON) <<<\n" : "DISABLED (OFF) <<<\n");
    printf(" > high voltage: %.3f V (x1000)\n", sc.hv);

    //write to file
    tstamp = time(NULL);
    fprintf(fp, "%.2f V %s", sc.hv*1000., asctime(localtime(&tstamp)));
  }
  else { printf(" @@@ CAN message receive error code: %d\n", rcvStat); }
  delay(3*MSEC);
  rcv_msg[0] = '\0';

  fclose(fp);

  return HV_EN;
}


void enableHV(int pwrEn)
{
  if (pwrEn) // ENABLE high voltage
  {
    char *can_msg[] = {"dummy", "can0", "040#BEEFDEAD00000000"};
    cansend(can_msg);
  }
  if (!pwrEn) // DISABLE high voltage
  {
    char *can_msg[] = {"dummy", "can0", "030#00000000BEEFDEAD"};
    cansend(can_msg);
  }
  printf("  The high voltage is >>> ");
  printf(pwrEn ? "ENABLED (ON) <<<\n" : "DISABLED (OFF) <<<\n");

  delay(100);
}


int setHV(float vset)
{
  if (vset > HV_MAX)
  {
    vset = HV_MAX;
  }

  float vpct = vset / C40N_MAX;
  printf("    fraction of max HV output (4kV) = %.5f\n", vpct);
  float dac_vout = vpct * DAC_VMAX;
  printf("    DAC output voltage = %.5f\n\n", dac_vout);

  // convert into DAC input code
  int k = 0;
  k = (int)(pow(2,12) * dac_vout / DAC_VREF);
  //printf("DEBUG:  k = %d,  k = %X (hex),  k << 3 = %X (hex)\n", k, k, (k<<3));

  // prepare CAN msg
  char msg[] = "050#0000000000000000";
  char tmp[17];
  sprintf(tmp, "%X\n", (k<<3));
  //printf("DEBUG:  msg = %s, len = %d\n", msg, strlen(msg));
  //printf("DEBUG:  tmp = %s, len = %d\n", tmp, strlen(tmp));
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
  printf("DEBUG:  can_msg[2] = %s\n\n", can_msg[2]);
  cansend(can_msg);
  delay(3*MSEC);

  return 0;
}
*/
