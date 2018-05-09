#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "iocontroller.h"												// Responsible for GPIO
#include "inputprocessing.h"											// Responsible for processing user's inputs

bool g_toggle = false;													// First state of the LED (off)
 
/** Responsible for processing user input.
	If the input is equal to the preset keyword, g_toggle on/off lights,
	depending on the current state.  
**/
void toggle_led(char buf[]) {
	int i = strcmp(buf, "g_toggle\n");									// [Debugging] Comparing the input string to the keyword.
	printf("%d\n", i);													
	if (0 == strcmp(buf, "g_toggle\n")) {									// If the input matches the preset keyword, turn on/off, depending on the state of the LED.
		if (g_toggle == false) {
			turn_on();
			g_toggle = true;
		}
		else {
			turn_off();
			g_toggle = false;
		} 
	}
}
