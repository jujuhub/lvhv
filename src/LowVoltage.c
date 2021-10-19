/*
 * LowVoltage.c
 *
 * purpose
 *    enables/disables the low voltage lines on LVHV board
 */

#include "LowVoltage.h"
#include "canlib.h"
#include "lib.h"


int main(void)
{
  printf("*---------*---------*---------*---------*---------*\n");
  printf("*-- This program controls the low voltage lines --*\n");
  printf("*---------*---------*---------*---------*---------*\n");
  printf("...\n Press Ctrl+C to Quit \n...\n");

  /* check that the CAN bus network is up */
  char user_input[NCHAR];
  bool canbusEnabled = false;

  while (!canbusEnabled)
  {
    printf("\nIs the CAN bus network up? [Y/N] > ");
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
      printf("\nQuitting program...\n");
      return 0;
    }
  } // end while loop

  /* check the low voltage status */
  checkLV();

  bool userDone = false;
  while (!userDone)
  {
    char user_input[NCHAR];
    char *tmp = NULL;
    long long userOption, pwrEn;

    printf("\nWhat would you like to do?\n");
    printf("  [1] Turn low voltage on/off\n");
    printf("  [2] Check status of low voltage\n");
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
        printf("Do you want to turn the low voltage ON(1) or OFF(0)? > ");
        fgets(user_input, NCHAR, stdin);
        pwrEn = strtol(user_input, &tmp, NCHAR);
        if (pwrEn != 0 && pwrEn != 1)
        {
          printf("Oopsies!\n");
          break;
        }
        powerLV(pwrEn);
        checkLV();
        break;

      case 2:
        checkLV();
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

void checkLV(void)
{
  //double vmon_3v3 = 0., vmon_2v5 = 0., vmon_1v2 = 0.;
  bool LV_EN = false;
  int rcvStat = -8;
  char rcv_msg[21];
  struct SlowControlsData sc;

  printf("\nChecking status of low voltage lines...\n");
  // send CAN msg to request low voltage status (ON/OFF)
  char *can_msg[] = {"dummy", "can0", "022#0000DEADBEEF0000"};
  cansend(can_msg);
  usleep(USLP);
  rcvStat = canread(rcv_msg);
  if (rcvStat == 1)
  {
    decodeCANmsg(&sc, rcv_msg);
    LV_EN = sc.lv_en;
    printf("  The low voltage is >>> ");
    printf(LV_EN ? "ENABLED (ON)\n" : "DISABLED (OFF)\n");
  }
  else { printf(" @@@ CAN message receive error code: %d\n", rcvStat); }
  delay(3*MSEC);
  rcv_msg[0] = '\0';

  printf("\nChecking values of low voltage lines...\n");
  // send CAN msg to request LV values (ADC)
  char *can_msg1[] = {"dummy", "can0", "3AD#00AD00AD00AD00AD"};
  cansend(can_msg1);
  usleep(USLP);
  rcvStat = canread(rcv_msg);
  if (rcvStat == 1)
  {
    decodeCANmsg(&sc, rcv_msg);
    printf(" > low voltages: %.1f V, %.1f V, %.f V.\n", sc.lvA, sc.lvB, sc.lvC);
  }
  else { printf(" @@@ CAN message receive error code: %d\n", rcvStat); }
  delay(3*MSEC);
  rcv_msg[0] = '\0';

  // convert ADC values
  //printf("  The low voltages are: \n");
  //printf("    3V3 line: %f V\n", vmon_3v3);
  //printf("    2V5 line: %f V\n", vmon_2v5);
  //printf("    1V2 line: %f V\n", vmon_1v2);
  //printf("\n");
}

void powerLV(int pwrEn)
{
  if (pwrEn) // turn ON low voltage
  {
    char *can_msg[] = {"dummy", "can0", "020#DEADBEEF00000000"};
    printf("Turning LV >>> ON <<<\n");
    cansend(can_msg);
  }
  if (!pwrEn) // turn OFF low voltage
  {
    char *can_msg[] = {"dummy", "can0", "010#00000000DEADBEEF"};
    printf("Turning LV >>> OFF <<<\n");
    cansend(can_msg);
  }

  delay(100);
}
