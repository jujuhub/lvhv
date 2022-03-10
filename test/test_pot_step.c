#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>

#define PIN_A 4   //BCM:23    //TODO:figure out pins!
#define PIN_B 5   //BCM:24

void discharge(void)
{
  pinMode(PIN_A, INPUT);
  pinMode(PIN_B, OUTPUT);
  digitalWrite(PIN_B, LOW);
  //delay by 5ms
  usleep(5000);
}

int charge_time(void)
{
  pinMode(PIN_B, INPUT);
  pinMode(PIN_A, OUTPUT);

  int count = 0;
  digitalWrite(PIN_A, HIGH);

  while (!digitalRead(PIN_B)) count++;
  return count;
}

int analog_read(void)
{
  discharge();
  return charge_time();
}

int main(void)
{
  //initialize
  wiringPiSetup();

  while (1)
  {
    int measurment = analog_read();
    printf("%d\n", measurement);
  }

  return 0;
}

