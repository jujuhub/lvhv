/*
 * PowerRelay.c
 *
 * purpose
 * 	  Control the power relay switch on the Raspberry Pi
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>

#include "PowerRelay.h"

int main(void)
{
  printf("*---------*---------*---------*---------*---------*\n");
  printf("*- This program controls power to the LVHV board -*\n");
  printf("*---------*---------*---------*---------*---------*\n");
  printf("...\n Press Ctrl+C to Quit \n...\n");

  initRelay();

  bool userDone = false;

  while (!userDone)
  {
    char user_input[NCHAR];
    char *tmp = NULL;
    long long userOption, rlyChn, pwrEn;
//    bool rlySwitched = false;

    printf("\nWhat would you like to do? \n");
    printf("  [1] Switch a relay (1/2/3) on/off\n");
    printf("  [2] Switch ALL relays on/off\n");
    printf("  [3] Check status of relays\n");
    printf("  [4] Nothing else\n");
    fgets(user_input, NCHAR, stdin);

    userOption = strtol(user_input, &tmp, NCHAR);
    if (userOption < 1 || userOption > NOPTIONS)
    {
      printf("Try again! \n");
      continue;
    }
    else
    {
      printf(" >>> You chose option [%lld] <<<\n\n", userOption);
    }

    switch(userOption)
    {
      case 1:
        printf("Which relay (1/2/3)? > ");
        fgets(user_input, NCHAR, stdin);
        rlyChn = strtol(user_input, &tmp, NCHAR);
        if (rlyChn < 1 || rlyChn > 3)
        {
          printf("Try again! \n");
          break;
        }
        printf(" ON(1) or OFF(0)? > ");
        fgets(user_input, NCHAR, stdin);
        pwrEn = strtol(user_input, &tmp, NCHAR);
        if (pwrEn != 0 && pwrEn != 1)
        {
          printf("Oopsies!\n");
          break;
        }
        powerRelay(rlyChn, pwrEn);
        break;

      case 2:
        printf("Switching all relays  ON(1) or OFF(0)? > ");
        fgets(user_input, NCHAR, stdin);
        pwrEn = strtol(user_input, &tmp, NCHAR);
        if (pwrEn != 0 && pwrEn != 1)
        {
          printf("Oopsies! \n");
          break;
        }
        for (rlyChn = 1; rlyChn < (MAX_NRLY+1); rlyChn++)
        {
          powerRelay(rlyChn, pwrEn);
        }
        break;

      case 3:
        checkRelay();
        break;

      case 4:
        userDone = true;
        printf("\nQuitting program.. \n");
        break;

      default :
        printf("What.. ? \n");
        break;
    } // end switch statement
  } // end while loop

  return 0;
}

void initRelay(void)
{
  wiringPiSetup();
  pinMode(RLY1, OUTPUT);
  pinMode(RLY2, OUTPUT);
  pinMode(RLY3, OUTPUT);

  checkRelay();
}

bool powerRelay(int rlyN, int rlyPWR)
{
  int rlyCH;
  bool rlySTAT = false;

  switch(rlyN)
  {
    case 1:
      rlyCH = RLY1; break;

    case 2:
      rlyCH = RLY2; break;

    case 3:
      rlyCH = RLY3; break;

    default :
      printf(" Invalid relay channel! \n");
      return rlySTAT;
  }

  digitalWrite(rlyCH, rlyPWR);
  delay(RLYDLY);
  if ((digitalRead(rlyCH)) == rlyPWR)
  {
    rlySTAT = true;
    printf("Relay %d: %d\n", rlyN, rlyPWR);
  }

  return rlySTAT;
}

void checkRelay(void)
{
  // check whether each relay is on or off

  int rlyEN;
  printf("Checking relays.. \n  0 = LOW/OFF\n  1 = HIGH/ON\n");

  rlyEN = digitalRead(RLY1);
  printf("Relay 1: %d\n", rlyEN);
  delay(RLYDLY);

  rlyEN = digitalRead(RLY2);
  printf("Relay 2: %d\n", rlyEN);
  delay(RLYDLY);

  rlyEN = digitalRead(RLY3);
  printf("Relay 3: %d\n", rlyEN);
  delay(RLYDLY);
}
