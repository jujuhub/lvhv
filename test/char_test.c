#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NCHAR 10

int main()
{
  char user_input[NCHAR];
  printf("Input a decimal value > ");
  fgets(user_input, NCHAR, stdin);

  // testing creation of can_msg from diff strings
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

  // replacing individual characters in a string
  char s[] = "123#0000000000000000";
  printf("s[4] = %c\n", s[4]);

  s[4] = '9';
  printf("s[4] = %c\n", s[4]);
  printf("s = %s, len = %d\n", s, strlen(s));

  // test time
  time_t tstamp;
  tstamp = time(NULL);
  printf("%s\n", asctime(localtime(&tstamp)));

  // test reading last line: this method cuts off the first character
  FILE *fprd = fopen("fprintf_test.txt", "r");
  char line[1024] = "";
  char c;
  int len = 0;
  if (fprd == NULL) return -1;
  fseek(fprd, -1, SEEK_END);
  c = fgetc(fprd);
  while (c == '\n')
  {
    fseek(fprd, -2, SEEK_CUR);
    c = fgetc(fprd);
    printf("while[1] c = %d, %c\n", c, c);
  }
  while (c != '\n')
  {
    fseek(fprd, -2, SEEK_CUR);
    len++;
    c = fgetc(fprd);
    printf("while[2] c = %d, %c\n", c, c);
  }
  fseek(fprd, 0, SEEK_CUR);
  if (fgets(line, len+1, fprd) != NULL) printf("%s, len = %d\n", line, strlen(line));
  else printf("Error\n");
  fclose(fprd);

  // extract first float value
  int i = 0;
  char fval[15] = "";
  for (i = 0; i < strlen(line); i++)
  {
    if (line[i] == ' ' && i != 0)
    {
      fval[i] = '\0';
      break;
    }
    else
    {
      fval[i] = line[i];
    }
  }

  printf("fval = %s, len = %d\n", fval, strlen(fval));

  // test reading last line: this method gives segfault
/*  FILE *fr = fopen("fprintf_test.txt", "rb");
  static const long max_len = 50;
  char buf[max_len+1];
  fseek(fr, -max_len, SEEK_END);
  size_t len = fread(buf, max_len, 1, fr);
  fclose(fr);
  buf[len] = '\0';
  char *last_newline = strrchr(buf, '\n');
  char *last_line = last_newline+1;
  printf("captured: %s\n", last_line);
*/

  // test writing to file
  FILE *fp = fopen("fprintf_test.txt", "a+");
  fprintf(fp, "%f - %s", d, asctime(localtime(&tstamp)));
  fclose(fp);

  return 0;
}
