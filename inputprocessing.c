#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iocontroller.h"												// Responsible for GPIO
#include "inputprocessing.h"											// Responsible for processing user's inputs

typedef enum {false, true} bool;

bool toggle = false;

// Responsible for processing user input
void process(char buf[]) {
	int i = strcmp(buf, "toggle\n");									// [Debugging] Comparing the input string to the keyword.
	printf("%d\n", i);													
	if (strcmp(buf, "toggle\n") == 0) {									// If the input matches the preset keyword, turn on/off, depending on the state of the LED.
		if (toggle == false){
			turn_on();
			toggle = true;
		}
		else {
			turn_off();
			toggle = false;
		}
	}  
}
