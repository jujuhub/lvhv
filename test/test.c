#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>

#define RLY1 25

bool isAlphaValid(char *usr_inp);

int main(int argc, char **argv)
{
  if (argc != 3)
  {
    printf("Incorrect syntax! Usage: ./prog can0 123#DEADBEEF00000000\n");
    return 1;
  }

  char *s[] = {"dummy", "can0", "123#DEADBEEF"};

  int rlyCh = RLY1;

  wiringPiSetup();
  pinMode(RLY1, OUTPUT);

  while(0)
  {
    digitalWrite(rlyCh, 1);
    delay(500);
    digitalWrite(25, 0);
    delay(500);
  }

  char user_input[10];

  printf("Please enter something> ");
  fgets(user_input, 10, stdin);

  if (!(isAlphaValid(user_input)))
  {
    printf("Please enter a valid response.\n");
  }

  return 0;

}

bool isAlphaValid(char *usr_inp)
{
  bool valid = true;
  int i, slen;
  slen = strlen(usr_inp);
  printf("Length of response: %d\n", slen);

  for (i = 0; i < (slen); i++)
  {
    if (isalpha(usr_inp[i]) || usr_inp[i] == '\n') { continue; }
    else { valid = false; }
  }

  return valid;
}
