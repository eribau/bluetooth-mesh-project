#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stdbool.h>
#include "iocontroller_rgb.h"

#define RED 0
#define GREEN 2
#define BLUE 3
#define HIGH 1
#define LOW 0
/****************************************
*wiringPi0 for red			*
*wiringPi2 for green			*
*wiringPi3 for blue			*
*					*
****************************************/
void init_gpio(){
	pinMode(RED, OUTPUT);
	digitalWrite(RED, LOW);
	pinMode(BLUE, OUTPUT);
	digitalWrite(BLUE, LOW);
	pinMode(GREEN, OUTPUT);
	digitalWrite(GREEN, LOW);
}

/** Sets io pin 11 to high **/
void red_on() {
        printf("Turning on led");
        if (wiringPiSetup() == -1) exit (1);
	pinMode(RED, OUTPUT);
	printf("RED On\n");
	digitalWrite(RED, HIGH);
}

/** Sets io pin 11 to low **/
void red_off() {
	printf("Turning off led");
	if (wiringPiSetup() == -1) exit (1);
	pinMode(RED, OUTPUT);
	printf("RED Off\n");
	digitalWrite(RED, LOW);
}

/** Sets io pin 13 to high **/
void green_on() {
	printf("Turning on led");
	if (wiringPiSetup() == -1) exit (1);
	pinMode(GREEN, OUTPUT);
	printf("LED On\n");
	digitalWrite(GREEN, HIGH);
}

/** Sets io pin 13 to low **/
void green_off() {
	printf("Turning off led");
	if (wiringPiSetup() == -1) exit (1);
	pinMode(GREEN, OUTPUT);
	printf("LED Off\n");
	digitalWrite(GREEN, LOW);
}

/** Sets io pin 15 to high **/
void blue_on(){
	printf("Turning on led");
	if (wiringPiSetup() == -1) exit (1);
	pinMode(BLUE, OUTPUT);
	printf("LED On\n");
	digitalWrite(BLUE, HIGH);
}

/** Sets pin 15 to low **/
void blue_off() {
	printf("Turning off led");
	if (wiringPiSetup() == -1) exit (1);
	pinMode(BLUE, OUTPUT);
	printf("LED Off\n");
	digitalWrite(BLUE, LOW);
}


