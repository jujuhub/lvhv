/*
 * lib.c
 *
 * purpose
 *    contains functions used in lvhv_routine.c
 */

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <wiringPi.h>

#include "lib.h"
#include "canlib.h"

bool isValidInput(char *usr_inp)
{
  bool valid = true;
  int i, slen;
  slen = strlen(usr_inp);

  if (slen == 1 || usr_inp[0] == '\n') // if 'enter' key was hit
  {
    printf("What was that?\n");
    valid = false;
    return valid;
  }

  if (slen > 10) // this doesn't occur bc of fgets
  {
    printf("Your response is too long (> 10).\n");
    valid = false;
    return valid;
  }

  for (i = 0; i < slen; i++)
  {
    if (isalpha(usr_inp[i]) || usr_inp[i] == '\n') { continue; }
    else { valid = false; }
  }

  return valid;
}

int checkYN(char *usr_inp)
{
  if (usr_inp[0] == 'Y' || usr_inp[0] == 'y') return 1;
  if (usr_inp[0] == 'N' || usr_inp[0] == 'n') return 0;
  else return 3;
}

float readPrevV(char *fname)
{
  FILE *fp = fopen(fname, "r");
  char line[1024]= "";
  char c;
  int len = 0;

  struct stat stat_record;
  if (stat(fname, &stat_record))
  {
    printf("%s", strerror(errno));
  }
  else if (stat_record.st_size <= 1)
  {
    printf(" %s is empty! Returning 0.\n", fname);
    return 0.;
  }

  if (fp == NULL)
  {
    printf("  !!! Could not open file!\n");
    return -999.;
  }

  fseek(fp, -1, SEEK_END);
  c = fgetc(fp);

  while (c == '\n')
  {
    fseek(fp, -2, SEEK_CUR);
    c = fgetc(fp);
//    printf("DEBUG:  while[1] c = %c (%d)\n", c, c);
  }

  while (c != '\n')
  {
    fseek(fp, -2, SEEK_CUR);
    ++len;
    c = fgetc(fp);
//    printf("DEBUG:  while[2] c = %c (%d)\n", c, c);
  }

  fseek(fp, 0, SEEK_CUR);

  if (fgets(line, len+1, fp) != NULL) puts(line);
  else printf("  !!! Error!\n");
  fclose(fp);

  // extract previously set voltage
  int i = 0;
  char vstr[15] = "";
  float vprev = 0.;

  for (i = 0; i < strlen(line); i++)
  {
    if (line[i] == ' ' && i != 0)
    {
      vstr[i] = '\0';
      break;
    }
    else
    {
      vstr[i] = line[i];
    }
  }

  vprev = (float)atof(vstr);
//  printf("DEBUG: readPrevV returns %f, vstr = %s\n", vprev, vstr);

  return vprev;
}

void fetchRHT(void)
{
  char *can_msg[] = {"dummy", "can0", "123#0000000000000000"};
  cansend(can_msg);
  delay(OPSDLY);
}

void statCheck(void)
{
  char user_input[10];
  bool INIT_STAT = false;

  while(!INIT_STAT)
  {
    printf("Do you wish to perform initial checks on the System? (Y/N)> ");
    fgets(user_input, 10, stdin);
    printf("Your answer> %s", user_input);
    if (!isValidInput(user_input))
    {
      printf("Please enter something valid.\n");
      continue;
    }

    if (checkYN(user_input))
    {
      printf("Fetching data... \n");
      // send CAN request to fetch RH&T data, light level data, previous 
      // voltage values (stored in a txt file), print info, and store this in
      // another txt file
      char *can_msg[] = {"dummy", "can0", "123#0000000000000000"};
      cansend(can_msg);

      printf("Data fetched!\n");
      INIT_STAT = true;
    }

    else
    {
      printf("Ok.. Your loss. If something breaks, it will be your fault...\n");
      while (!checkYN(user_input))
      {
        printf("Are you SURE? (Y/N)> ");
        fgets(user_input, 10, stdin);
        printf("Your response> %s", user_input);
        if (!isValidInput(user_input))
        {
          printf("Please enter a valid response.\n");
          continue;
        }

        if (checkYN(user_input))
        {
          printf("OK. I hope you know what you're doing...\n");
          INIT_STAT = true;
          break;
        }

        else
        {
          printf("I'm glad you changed your mind! Fetching data now.. ");
          // fetch data
          printf("Data fetched!");
          INIT_STAT = true;
          break;
        }
      } // end inner INIT_STAT while loop
    }
  } // end INIT_STAT while loop
}


/**************************************************
 *                NOT USED ANYMORE                *
 **************************************************/

void initSequence(void) // NOT USED ANYMORE
{
  char user_input[10];

  /* POWER UP THE LVHV BOARD (SWITCH POWER RELAY) */
  bool PWR_EN = false;

  while (!PWR_EN)
  {
    printf("Do you wish to power up (+12V) the LVHV board? (Y/N)> ");
    fgets(user_input, 10, stdin);
    printf("Your answer> %s", user_input);
    if (!isValidInput(user_input))
    {
      printf("Please enter something valid!\n");
      continue;
    }

    // nuances: any word beginning with Y/y or N/n will make a decision!
    if (checkYN(user_input))
    {
      /* switch power relay ON */
      long long rlyN = 0; // default: all channels lol
      char rly_input[10];
      char *tmp = NULL;

      printf("Which relay do you wish to switch? (1/2/3/0[all])> ");
      fgets(rly_input, 10, stdin);
      rlyN = strtol(rly_input, &tmp, 10);
      printf("Relay Ch: %lld\n", rlyN);

      relayHAT(rlyN, HIGH);

      PWR_EN = true;
      printf("The LVHV is powered up. You can communicate with it now.\n");
    }
    else
    {
      printf("You cannot do anything until the LVHV board is powered up.\n");
      // wait for user to say YES
    }
  } // end PWR_EN while loop


  /* TURN ON LOW VOLTAGE LINES */
  bool LV_EN = false;

  while(!LV_EN)
  {
    printf("Do you wish to turn ON the low voltage lines? (Y/N)> ");
    fgets(user_input, 10, stdin);
    printf("Your final answer> %s", user_input);
    if (!isValidInput(user_input))
    {
      printf("Please enter a valid response.\n");
      continue;
    }
    else if (checkYN(user_input))
    {
      /* send CAN msg to turn ON LV lines */
      // msgID = 0x020
      char *can_msg[] = {"dummy", "can0", "020#DEADBEEF00000000"};

      cansend(can_msg);
      delay(100);

//      canread();
//      for (int i = 0; i < 50; ++i);

      LV_EN = true;
      printf("The low voltage lines are ON.\nYou can now (1) read humidity and temperature, \n(2) set and turn ON/OFF the high voltage, \n(3) set the trigger threshold. \n");
    }
    else
    {
      printf("ok.. :( \nYou can't do much else here either...\n");
      // wait until user says YES
    }
  } // end LV_EN while loop
}

void initRelayHAT(void)
{
  wiringPiSetup();
  pinMode(RLY1, OUTPUT);
  pinMode(RLY2, OUTPUT);
  pinMode(RLY3, OUTPUT);

  // make sure they switched OFF
  digitalWrite(RLY1, LOW);
  delay(RLYDLY);
  digitalWrite(RLY2, LOW);
  delay(RLYDLY);
  digitalWrite(RLY3, LOW);
  delay(RLYDLY);
}

/* turn on/off power relay HAT */
void relayHAT(int rlyN, int rlyEN)
{
  int rlyCH;

  if (rlyN == 1) rlyCH = RLY1;
  if (rlyN == 2) rlyCH = RLY2;
  if (rlyN == 3) rlyCH = RLY3;

  if (rlyN == 0)
  {
    digitalWrite(RLY1, rlyEN);
    delay(RLYDLY);
    digitalWrite(RLY2, rlyEN);
    delay(RLYDLY);
    digitalWrite(RLY3, rlyEN);
    delay(RLYDLY);
    return;
  }
  else
  {
    digitalWrite(rlyCH, rlyEN);
    delay(RLYDLY);
    return;
  }
}
