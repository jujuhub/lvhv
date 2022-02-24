/*
 * SlowControl.c
 *
 * written by: J. He
 *
 * purpose
 *    manages the slow controls routine of the LVHV board
 *      - humidity, temperature
 *      - photodiode
 *      - low voltages
 *      - high voltage
 *      - trigger board thresholds
 * usage
 *    user just needs to execute the program and let it run
 *      ./SlowControl
 */

#include "lib.h"
#include "canlib.h"

#define FILEPATH "/home/pi/can-/candump*.log"
#define USE_CANLOGS 0 //bool

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
      printf("  ok \n");
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
      printf("  !!! Please enter a valid response \n");
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


  /*** check if trigger bd is present ***/
  bool trigBd_ok = false;
  bool hasTrigBd = false;

  while (!trigBd_ok)
  {
    printf("Is the trigger board connected to the LVHV? [Y/N] > ");
    fgets(user_input, 10, stdin);
    if (!isValidInput(user_input))
    {
      printf("  !!! Please enter a valid answer \n");
      continue;
    }

    if (checkYN(user_input) != 0 && checkYN(user_input) != 1)
    {
      printf("  Oopsies! Try again :)\n");
      continue;
    }
    if (checkYN(user_input) == 1)
    {
      printf("  got it!\n\n");
      hasTrigBd = true;
      trigBd_ok = true;
    }
    else if (checkYN(user_input) == 0)
    {
      printf("  got it!\n\n");
      trigBd_ok = true;
    }
  }

// if (RHT == 'OK')
//   proceed with routine
// if (RHT != 'OK')
//  printf("WARNING!! RH&T levels are above threshold! Do you want to continue? (Y/N)> ");
  // wait for user input
  // keep reading RH&T until levels are OK?

// after status checks come out OK
//  printf("Do you wish to turn on the HV? (Y/N)> ");
// wait for user input
// if YES
//    start HV turn on sequence.. call on a script?
//    set HV_ON_OFF logic to be HIGH, but first set DAC output to be 0V
// if NO
//    wait until user says yes
//    or move on to checking RH&T

  // read from canlogs instead of receiving can msgs directly
  // initialize list of canlogs
  bool use_canlogs = USE_CANLOGS;
  glob_t globlist;
  glob(FILEPATH, GLOB_TILDE, NULL, &globlist);
  int nlogs = 0;

  nlogs = globlist.gl_pathc;
  printf(" >> num of canlogs found: %d\n", nlogs);
  if (nlogs > 0) { printf(" >> most recent canlog: %s\n", globlist.gl_pathv[globlist.gl_pathc-1]); }
  else { printf(" >> did not find any canlogs !\n"); }
  //char mainfile[] = "";
  //strcpy(mainfile, globlist.gl_pathv[globlist.gl_pathc-1]);


  /*** test struct ***/
  //char dummy_msg[21] = "321#0153286D3A000000";  //rh&t
  //char dummy_msg[21] = "220#0001000100010001";  //lv en
  //char dummy_msg[21] = "3DA#02EF00026C000162";  //lv adc
  //dummy_msg[20] = '\0';
  struct SlowControlsData sc;

  //decodeCANmsg(&sc, dummy_msg);
  //printf(" ~~~ test ~~~\n");
  //printf(" humidity: %.1f %%\n temperature: %.1f deg C\n", sc.hum, sc.temp);
  //printf(" low voltage is: ");
  //printf(sc.lv_en ? "enabled\n" : "disabled\n");
  //printf(" low voltages are: %0.2f V, %0.2f V, %0.2f V\n", sc.lvA, sc.lvB, sc.lvC);

  int rcvStat = -8;  //can msg receive status
  char rcv_msg[21];
  //bool dataFetched = false;
  while (1)
  {
    if (use_canlogs)
    {
      glob(FILEPATH, GLOB_TILDE, NULL, &globlist); //GLOB_APPEND will not sort entire list
      if ((nlogs < globlist.gl_pathc))
      {
        nlogs = globlist.gl_pathc;    // update the number of canlogs
        printf(" >> num of canlogs found: %d\n", globlist.gl_pathc);
        printf(" >> most recent canlog: %s\n", globlist.gl_pathv[globlist.gl_pathc-1]);

      /*  int i; for (i = 0; i < globlist.gl_pathc; ++i) //DEBUG
        {
          printf("%s\n", globlist.gl_pathv[i]);
        } */
      }

      // read from most recent canlog
      readcanlog(globlist.gl_pathv[globlist.gl_pathc-1]);
    }

    /*** fetch humidity & temp data ***/
    printf("\nFetching RH&T data...\n");
    //send CAN msg to request RH&T
    char *can_msg_rht[] = {"dummy", "can0", "123#0000000000000000"};
    cansend(can_msg_rht);
//    delay(1*MSEC); // need this delay to read data
    usleep(5*USLP);
    rcvStat = canread(rcv_msg);
    //printf("rcv_msg: %s \nlength of rcv_msg: %u\n", rcv_msg, strlen(rcv_msg)); //DEBUG
    if (rcvStat == 1)
    {
      decodeCANmsg(&sc, rcv_msg);
      printf(" > humidity: %.1f %%\n > temperature: %.1f deg C\n", sc.hum, sc.temp);
    }
    else { printf(" @@@ CAN message receive error code: %d\n", rcvStat); }
    delay(3*MSEC);
    rcv_msg[0] = '\0';

    // receive CAN msg with RH&T data
    // convert to human-readable format
    // save in .txt file: data status ('Normal' or 'Stale')
    //                    RH in decimal
    //                    T in decimal
    //                    data type (from RPi script or user command?)
    // keep running average of RH&T (separate script?)
    // probe for user input?


    /*** fetch photodiode data ***/
    printf("Fetching light levels...\n");
    char *can_msg_pd[] = {"dummy", "can0", "00D#000D000D000D000D"};
    cansend(can_msg_pd);
    usleep(USLP);
    rcvStat = canread(rcv_msg);
    if (rcvStat == 1)
    {
      decodeCANmsg(&sc, rcv_msg);
      printf(" > photodiode: %.2f %%\n", sc.photodiode);
    }
    else { printf(" @@@ CAN message receive error code: %d\n", rcvStat); }
    delay(3*MSEC);
    rcv_msg[0] = '\0';


    /*** read back low voltages ***/
    printf("Fetching low voltage values...\n");
    char *can_msg_lv[] = {"dummy", "can0", "3AD#00AD00AD00AD00AD"};
    cansend(can_msg_lv);
    usleep(USLP);
    rcvStat = canread(rcv_msg);
    if (rcvStat == 1)
    {
      decodeCANmsg(&sc, rcv_msg);
      printf(" > low voltages: %.3f V, %.3f V, %.3f V.\n", sc.lvA, sc.lvB, sc.lvC);
    }
    else { printf(" @@@ CAN message receive error code: %d\n", rcvStat); }
    delay(3*MSEC);
    rcv_msg[0] = '\0';


    /*** check high voltage ***/
    printf("Fetching high voltage status...\n");
    char *can_msg_hv[] = {"dummy", "can0", "034#0000000000000000"};
    cansend(can_msg_hv);
    usleep(USLP);
    rcvStat = canread(rcv_msg);
    if (rcvStat == 1)
    {
      decodeCANmsg(&sc, rcv_msg);
      printf(" > high voltage: %.3f V (x1000)\n", sc.hv);
    }
    else { printf(" @@@ CAN message receive error code: %d\n", rcvStat); }
    delay(3*MSEC);
    rcv_msg[0] = '\0';


    /*** read back trigger board thresholds ***/ 
    if (hasTrigBd)
    {
      printf("Fetching trigger board threshold voltages...\n");
      char *can_msg_dac0[] = {"dummy", "can0", "0BC#0000000000000000"}; 
      cansend(can_msg_dac0);
      usleep(USLP);
      rcvStat = canread(rcv_msg);
      if (rcvStat == 1)
      {
        decodeCANmsg(&sc, rcv_msg);
        printf(" > trigger bd DAC0: %.3f V\n", sc.trig_dac0);
      }
      else { printf(" @@@ CAN message receive error code: %d\n", rcvStat); }
      delay(3*MSEC);
      rcv_msg[0] = '\0';

      char *can_msg_dac1[] = {"dummy", "can0", "0EF#0000000000000000"};
      cansend(can_msg_dac1);
      usleep(USLP);
      rcvStat = canread(rcv_msg);
      if (rcvStat == 1)
      {
        decodeCANmsg(&sc, rcv_msg);
        printf(" > trigger bd DAC1: %.3f V\n", sc.trig_dac1);
      }
      else { printf(" @@@ CAN message receive error code: %d\n", rcvStat); }
      delay(3*MSEC);
      rcv_msg[0] = '\0';
    }

    delay(7*MSEC);
    delay(10*MSEC);
    delay(10*MSEC);
    delay(10*MSEC);
    delay(10*MSEC);

  } // end main loop

  return 0;
}

/* ******************************
 * functions to test
 * ******************************/
