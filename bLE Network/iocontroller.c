#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <wiringPi.h>

#include "iocontroller.h"												// Responsible for GPIO

extern bool g_connection_check;											// Global variable from tcpserver

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
