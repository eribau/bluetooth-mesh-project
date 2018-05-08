#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <wiringPi.h>

#include "iocontroller.h"													// Responsible for GPIO

#define RED 0
#define GREEN 2
#define BLUE 3
#define HIGH 1
#define LOW 0

bool g_connection_check;											// Global variable from tcpserver

bool g_toggle = false;													// First state of the LED (off)
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

/** Responsible for processing user input.
	If the input is equal to the preset keyword, g_toggle on/off lights,
	depending on the current state.  
**/


/** Turns on LED at a specific preprogrammed pin **/
void turn_on() {														
	int pin = 7;														// LED's pin number
	printf("Turning on led");
	if (-1 == wiringPiSetup()) exit (1);								// Initialization of wiringPi pin numbering scheme
	pinMode(pin, OUTPUT);												// Setting pin as the output
	printf("LED On\n");
	digitalWrite(pin, 1);												// Sending signal to the pin
}

/** Turns off LED at a specific preprogrammed pin **/
void turn_off() {														// Code for this method follows the turn_on documentation
	int pin = 7;														
	printf("Turning off led");
	if (-1 == wiringPiSetup()) exit (1);
	pinMode(pin, OUTPUT);
	printf("LED Off\n");
	digitalWrite(pin, 0);
}

void toggle_led() {
								// If the input matches the preset keyword, turn on/off, depending on the state of the LED.
	if (g_toggle == false) {
		turn_on();
		g_toggle = true;
	}
	else {
		turn_off();
		g_toggle = false;
	} 
	
}

/** 
   Responsible for processing sensor input (button in this case).
   When button pressed, send messages to server and client
**/
void button(int client_fd) {											
	wiringPiSetup();
	pinMode(2, INPUT);													// Sets button as an input
	int prev_button = HIGH;												// Last state of the pull-up circuit
	while (1) {
		if(false == g_connection_check) exit(0);							// If connection to the client was closed, terminate
		if(prev_button == HIGH && digitalRead(2) == LOW) {				// A falling edge
			prev_button = LOW;
			char reply[] = "button pressed\n";
			send(client_fd, reply, sizeof(reply), 0);					// [Debugging] Send message to client
			printf("Server message\n");									// [Debugging] Send message to server
		}else if(prev_button == LOW && digitalRead(2) == HIGH) {			// a rising edge, do nothing
			prev_button = HIGH;
		}
		delay(100);
	}
}
