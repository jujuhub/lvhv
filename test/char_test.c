#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NCHAR 10

int main()
{
  char user_input[NCHAR];
  printf("Input a decimal value > ");
  fgets(user_input, NCHAR, stdin);

  char *can_msg[] = { "dummy", "can0", user_input };
  printf("\nuser_input = %s\n", can_msg[2]);

  float f = 0.;
  f = (float)atof(can_msg[2]);
  printf("f = %f\n", f);

  float d;
  d = f - 100.;
  printf("d = %f\n", d);

  int p = (int)pow(2,12);
  printf("p = %d\n", p);

// below gives seg fault
//  can_msg[0][1] = 'i';
//  printf("new can_msg[0] = %s\n", can_msg[0]);

  return 0;
}

void char_test(char *cs)
{
  printf("\n");
}
