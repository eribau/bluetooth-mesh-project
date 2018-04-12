#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "iocontroller.h"												// Responsible for GPIO

typedef enum {false, true} bool;

extern bool connection_check;

/** Turns on LED at a specific preprogrammed pin **/
void turn_on() {														
    int pin = 7;														// LED's pin number
    printf("Turning on led");
    if (wiringPiSetup() == -1) exit (1);								// Initialization of wiringPi pin numbering scheme
    pinMode(pin, OUTPUT);												// Setting pin as the output
    printf("LED On\n");
    digitalWrite(pin, 1);												// Sending signal to the pin
}

/** Turns off LED at a specific preprogrammed pin **/
void turn_off() {														// Code for this method follows the turn_on documentation
    int pin = 7;														
    printf("Turning off led");
    if (wiringPiSetup() == -1) exit (1);
    pinMode(pin, OUTPUT);
    printf("LED Off\n");
    digitalWrite(pin, 0);
}

/** Responsible for processing sensor input (button in this case).
 *  When button pressed, send messages to server and client **/
void button(int client_fd) {											
    wiringPiSetup();
    pinMode (2, INPUT);
    int prev_button = HIGH;												// Last state of the pull-up circuit
    while (1) {
        if(connection_check == false) exit(0);							// If connection to the client was closed, terminate
        if(prev_button == HIGH && digitalRead(2) == LOW) {				// A falling edge
            prev_button = LOW;
            send(client_fd, "Client message\n", 6, 0);					// [Debugging] Send message to client
            printf("Server message\n");									// [Debugging] Send message to server
        }
        else if(prev_button == LOW && digitalRead(2) == HIGH) {			// a rising edge, do nothing
            prev_button = HIGH;
        }
        delay(100);
    }
}
