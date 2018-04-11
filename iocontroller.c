#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include "iocontroller.h"												// Responsible for GPIO

void turn_on()															// Turn on LED at a specific pin
{
  int pin = 7;															// LED's pin number
  printf("Turning on led");

  if (wiringPiSetup() == -1)											// Initialization of wiringPi pin numbering scheme
    exit (1);

  pinMode(pin, OUTPUT);													// Setting pin as the output

    printf("LED On\n");
    digitalWrite(pin, 1);												// Sending signal to the pin
  
}

void turn_off()															// Responsible for GPIO
{																		// Code below follows the turn_on documentation
  int pin = 7;
	printf("Turning off led");

  if (wiringPiSetup() == -1)
    exit (1);

  pinMode(pin, OUTPUT);
  
    printf("LED Off\n");
    digitalWrite(pin, 0);
  
}
